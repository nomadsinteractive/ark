#include "dear-imgui/renderer/renderer_imgui.h"

#include <cctype>

#include <imgui.h>

#include "core/ark.h"
#include "core/types/global.h"
#include "core/inf/asset.h"
#include "core/inf/readable.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/camera.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/inf/render_command.h"
#include "graphics/impl/renderer/render_group.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/render_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/render_engine.h"
#include "renderer/inf/uploader.h"

#include "app/base/application_context.h"
#include "app/base/event.h"

#include "dear-imgui/base/imgui_context.h"
#include "dear-imgui/base/renderer_context.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

namespace {

class ImguiRenderCommand : public RenderCommand {
public:
    ImguiRenderCommand(sp<RenderCommand> delegate, sp<RendererImgui::DrawCommandRecycler> recycler)
        : _delegate(std::move(delegate)), _recycler(std::move(recycler)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _delegate->draw(graphicsContext);
    }

private:
    sp<RenderCommand> _delegate;
    sp<RendererImgui::DrawCommandRecycler> _recycler;
};

}

static void updateKeyStatus(ImGuiIO& io, ImGuiKey_ keycode, bool isKeyDown) {
    io.KeyMap[keycode] = isKeyDown ? keycode : -1;
    io.KeysDown[keycode] = isKeyDown;
}

RendererImgui::RendererImgui(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Shader>& shader, const sp<Texture>& texture)
    : _shader(shader), _render_controller(resourceLoaderContext->renderController()), _render_engine(_render_controller->renderEngine()), _renderer_group(sp<RendererGroup>::make()), _texture(texture),
      _pipeline_factory(shader->pipelineFactory()), _renderer_context(sp<RendererContext>::make())
{
}

void RendererImgui::render(RenderRequest& renderRequest, const V3& position)
{
    ImGuiIO& io = ImGui::GetIO();

    io.DisplaySize.x = _render_engine->viewport().width();
    io.DisplaySize.y = _render_engine->viewport().height();

    ImGui::NewFrame();
    _renderer_group->render(renderRequest, position);
    ImGui::EndFrame();
    ImGui::Render();
    MyImGuiRenderFunction(renderRequest, ImGui::GetDrawData());
}

void RendererImgui::addRenderer(const sp<Renderer>& renderer)
{
    _renderer_group->addRenderer(renderer);
}

bool RendererImgui::onEvent(const Event& event)
{
    ImGuiIO& io = ImGui::GetIO();
    const size_t MouseIndex[5] = {0, 2, 1, 0, 0};
    switch(event.action())
    {
    case Event::ACTION_MOVE:
        io.MousePos = ImVec2(event.x(), io.DisplaySize.y - event.y());
        return true;
    case Event::ACTION_DOWN:
        io.MouseDown[MouseIndex[event.code() - Event::CODE_MOUSE_BUTTON_LEFT]] = true;
        return true;
    case Event::ACTION_UP:
        io.MouseDown[MouseIndex[event.code() - Event::CODE_MOUSE_BUTTON_LEFT]] = false;
        return true;
    case Event::ACTION_WHEEL:
        io.MouseWheel = event.y();
        return true;
    case Event::ACTION_KEY_DOWN:
    case Event::ACTION_KEY_UP:
        {
            const bool isKeyDown = event.action() == Event::ACTION_KEY_DOWN;
            switch(event.code())
            {
                case Event::CODE_KEYBOARD_LSHIFT:
                case Event::CODE_KEYBOARD_RSHIFT:
                    io.KeyShift = event.action() == Event::ACTION_KEY_DOWN;
                    return true;
                case Event::CODE_KEYBOARD_LEFT:
                    updateKeyStatus(io, ImGuiKey_LeftArrow, isKeyDown);
                    return true;
                case Event::CODE_KEYBOARD_RIGHT:
                    updateKeyStatus(io, ImGuiKey_RightArrow, isKeyDown);
                    return true;
                case Event::CODE_KEYBOARD_UP:
                    updateKeyStatus(io, ImGuiKey_UpArrow, isKeyDown);
                    return true;
                case Event::CODE_KEYBOARD_DOWN:
                    updateKeyStatus(io, ImGuiKey_DownArrow, isKeyDown);
                    return true;
                case Event::CODE_KEYBOARD_DELETE:
                    updateKeyStatus(io, ImGuiKey_Delete, isKeyDown);
                    return true;
                case Event::CODE_KEYBOARD_BACKSPACE:
                    updateKeyStatus(io, ImGuiKey_Backspace, isKeyDown);
                    return true;
                default:
                    break;
            }
            if(!isKeyDown)
                break;
        }
    case Event::ACTION_KEY_REPEAT:
        if(event.code() < Event::CODE_NO_ASCII) {
            wchar_t c = Event::toCharacter(event.code());
            io.AddInputCharacter(static_cast<ImWchar>(io.KeyShift ? std::toupper(c) : c));
        }
        return true;
    default:
        break;
    }
    return false;
}

const sp<RendererContext>& RendererImgui::rendererContext() const
{
    return _renderer_context;
}

void RendererImgui::MyImGuiRenderFunction(RenderRequest& renderRequest, ImDrawData* draw_data)
{
    std::map<void*, sp<DrawCommandRecycler>> recyclers;

    for (int i = 0; i < draw_data->CmdListsCount; i++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[i];

        bytearray vb = _memory_pool.allocate(static_cast<size_t>(cmd_list->VtxBuffer.size_in_bytes()));
        bytearray ib = _memory_pool.allocate(static_cast<size_t>(cmd_list->IdxBuffer.size_in_bytes()));

        memcpy(vb->buf(), cmd_list->VtxBuffer.Data, static_cast<size_t>(cmd_list->VtxBuffer.size_in_bytes()));
        memcpy(ib->buf(), cmd_list->IdxBuffer.Data, static_cast<size_t>(cmd_list->IdxBuffer.size_in_bytes()));

        const sp<Uploader::Array<uint8_t>> verticsUploader = sp<Uploader::Array<uint8_t>>::make(vb);
        const sp<Uploader::Array<uint8_t>> indicesUploader = sp<Uploader::Array<uint8_t>>::make(ib);

        uint32_t offset = 0;
        const std::vector<RenderLayer::UBOSnapshot> ubos = _shader->snapshot(renderRequest);
        for (int j = 0; j < cmd_list->CmdBuffer.Size; j++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                // The texture for the draw call is specified by pcmd->TextureId.
                // The vast majority of draw calls will use the imgui texture atlas, which value you have set yourself during initialization.

                // We are using scissoring to clip some objects. All low-level graphics API should supports it.
                // - If your engine doesn't support scissoring yet, you may ignore this at first. You will get some small glitches
                //   (some elements visible outside their bounds) but you can fix that once everything else works!
                // - Clipping coordinates are provided in imgui coordinates space (from draw_data->DisplayPos to draw_data->DisplayPos + draw_data->DisplaySize)
                //   In a single viewport application, draw_data->DisplayPos will always be (0,0) and draw_data->DisplaySize will always be == io.DisplaySize.
                //   However, in the interest of supporting multi-viewport applications in the future (see 'viewport' branch on github),
                //   always subtract draw_data->DisplayPos from clipping bounds to convert them to your viewport space.
                // - Note that pcmd->ClipRect contains Min+Max bounds. Some graphics API may use Min+Max, other may use Min+Size (size being Max-Min)
                // Render 'pcmd->ElemCount/3' indexed triangles.
                // By default the indices ImDrawIdx are 16-bits, you can change them to 32-bits in imconfig.h if your engine doesn't support 16-bits indices.
                const ImVec2& pos = draw_data->DisplayPos;

                sp<DrawCommandRecycler> recycler = obtainDrawCommandRecycler(reinterpret_cast<Texture*>(pcmd->TextureId), recyclers);
                const sp<DrawCommand>& drawCommand = recycler->drawCommand();
                Buffer::Snapshot vertexBuffer = drawCommand->_vertex_buffer.snapshot(verticsUploader);
                Buffer::Snapshot indexBuffer = drawCommand->_index_buffer.snapshot(indicesUploader);
                DrawingContext drawingContext(_shader, drawCommand->_shader_bindings, ubos, std::move(vertexBuffer), std::move(indexBuffer), static_cast<int32_t>(pcmd->ElemCount / 3), offset, pcmd->ElemCount);
                drawingContext._parameters._scissor = _render_engine->toRendererScissor(Rect(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y, pcmd->ClipRect.z - pos.x, pcmd->ClipRect.w - pos.y), Ark::COORDINATE_SYSTEM_LHS);
                renderRequest.addRequest(sp<ImguiRenderCommand>::make(drawingContext.toRenderCommand(), std::move(recycler)));
            }
            offset += pcmd->ElemCount;
        }
    }
}

sp<RendererImgui::DrawCommandRecycler> RendererImgui::obtainDrawCommandRecycler(Texture* texture, std::map<void*, sp<RendererImgui::DrawCommandRecycler>>& cache)
{
    const auto iter = cache.find(texture);
    if(iter != cache.end())
        return iter->second;

    const sp<LFStack<sp<RendererImgui::DrawCommand>>>& drawCommandPool = _renderer_context->obtainDrawCommandPool(texture);

    sp<DrawCommand> drawCommand;
    if(!drawCommandPool->pop(drawCommand))
        drawCommand = sp<DrawCommand>::make(_shader, _pipeline_factory, _render_controller, texture ? sp<Texture>::make(*texture) : _texture);

    sp<DrawCommandRecycler> recycler = sp<DrawCommandRecycler>::make(drawCommandPool, drawCommand);
    cache.insert(std::make_pair(texture, recycler));
    return recycler;
}

RendererImgui::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _camera(sp<Camera>::make()), _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/imgui.vert", "shaders/imgui.frag", _camera))
{
}

sp<Renderer> RendererImgui::BUILDER::build(const Scope& args)
{
    const Viewport& viewport = _resource_loader_context->renderController()->renderEngine()->viewport();

    Ark::RendererCoordinateSystem coordinateSystem = _resource_loader_context->renderController()->renderEngine()->renderContext()->coordinateSystem();
    _camera->ortho(0, viewport.width(), 0, viewport.height(), viewport.near(), viewport.far(), static_cast<Ark::RendererCoordinateSystem>(-coordinateSystem));

    const Global<ImguiContext> context;
    ImGuiIO& io = ImGui::GetIO();

    for(const document& i : _manifest->children("font"))
    {
        ImFontConfig fontConfig;
        const String& src = Documents::getAttribute(i, Constants::Attributes::SRC);
        fontConfig.SizePixels = Documents::getAttribute<float>(i, Constants::Attributes::SIZE, 18.0f);
        if(src.empty())
            io.Fonts->AddFontDefault(&fontConfig);
        else
        {
            const sp<Asset> asset = Ark::instance().getAsset(src);
            DCHECK(asset, "Font \"%s\" not found", src.c_str());
            const sp<Readable> readable = asset->open();
            int32_t datasize = readable->remaining();
            void* data = malloc(static_cast<size_t>(datasize));
            readable->read(data, static_cast<uint32_t>(datasize));
            strncpy(fontConfig.Name, src.c_str(), sizeof(fontConfig.Name));
            io.Fonts->AddFontFromMemoryTTF(data, datasize, fontConfig.SizePixels, &fontConfig);
        }
    }

    unsigned char* pixels = nullptr;
    int32_t width, height, bytesPerPixel;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel);
    const sp<Bitmap> bitmap = sp<Bitmap>::make(width, height, bytesPerPixel * width, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(pixels), width * height * bytesPerPixel));
    const sp<Texture> texture = _resource_loader_context->renderController()->createTexture2D(sp<Size>::make(static_cast<float>(width), static_cast<float>(height)), sp<Texture::UploaderBitmap>::make(bitmap));

    return sp<RendererImgui>::make(_resource_loader_context, _shader->build(args), texture);
}

RendererImgui::DrawCommand::DrawCommand(const Shader& shader, const sp<PipelineFactory>& pipelineFactory, RenderController& renderController, const sp<Texture>& texture)
    : _vertex_buffer(renderController.makeVertexBuffer()), _index_buffer(renderController.makeIndexBuffer()),
      _shader_bindings(sp<ShaderBindings>::make(pipelineFactory, sp<PipelineBindings>::make(PipelineBindings::Parameters(ModelLoader::RENDER_MODE_TRIANGLES, Rect(), PipelineBindings::FLAG_CULL_MODE_NONE | PipelineBindings::FLAG_DYNAMIC_SCISSOR), shader.layout()), renderController, _vertex_buffer, _index_buffer))
{
    PipelineBindings& pipelineBindings = _shader_bindings->pipelineBindings();
    pipelineBindings.bindSampler(texture);
}

RendererImgui::DrawCommandRecycler::DrawCommandRecycler(const sp<LFStack<sp<RendererImgui::DrawCommand>>>& recycler, const sp<RendererImgui::DrawCommand>& drawCommand)
    : _recycler(recycler), _draw_command(drawCommand)
{
}

RendererImgui::DrawCommandRecycler::~DrawCommandRecycler()
{
    _recycler->push(_draw_command);
}

const sp<RendererImgui::DrawCommand>& RendererImgui::DrawCommandRecycler::drawCommand() const
{
    return _draw_command;
}

}
}
}
