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
#include "renderer/base/render_engine_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/render_engine.h"

#include "app/base/application_context.h"
#include "app/base/event.h"

#include "dear-imgui/base/draw_command_pool.h"
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

static ImGuiKey toImGuiKey(Event::Code code) {
    switch(code)
    {
        case Event::CODE_KEYBOARD_LSHIFT:
            return ImGuiKey_LeftShift;
        case Event::CODE_KEYBOARD_RSHIFT:
            return ImGuiKey_RightShift;
        case Event::CODE_KEYBOARD_LCTRL:
            return ImGuiKey_LeftCtrl;
        case Event::CODE_KEYBOARD_RCTRL:
            return ImGuiKey_RightCtrl;
        case Event::CODE_KEYBOARD_LALT:
            return ImGuiKey_LeftAlt;
        case Event::CODE_KEYBOARD_RALT:
            return ImGuiKey_RightCtrl;
        case Event::CODE_KEYBOARD_TAB:
            return ImGuiKey_Tab;
        case Event::CODE_KEYBOARD_A:
            return ImGuiKey_A;
        case Event::CODE_KEYBOARD_C:
            return ImGuiKey_C;
        case Event::CODE_KEYBOARD_V:
            return ImGuiKey_V;
        case Event::CODE_KEYBOARD_X:
            return ImGuiKey_X;
        case Event::CODE_KEYBOARD_Y:
            return ImGuiKey_Y;
        case Event::CODE_KEYBOARD_Z:
            return ImGuiKey_Z;
        case Event::CODE_KEYBOARD_LEFT:
            return ImGuiKey_LeftArrow;
        case Event::CODE_KEYBOARD_RIGHT:
            return ImGuiKey_RightArrow;
        case Event::CODE_KEYBOARD_UP:
            return ImGuiKey_UpArrow;
        case Event::CODE_KEYBOARD_DOWN:
            return ImGuiKey_DownArrow;
        case Event::CODE_KEYBOARD_DELETE:
            return ImGuiKey_Delete;
        case Event::CODE_KEYBOARD_BACKSPACE:
            return ImGuiKey_Backspace;
        case Event::CODE_KEYBOARD_ENTER:
            return ImGuiKey_Enter;
        case Event::CODE_KEYBOARD_INSERT:
            return ImGuiKey_Insert;
        case Event::CODE_KEYBOARD_PAGE_UP:
            return ImGuiKey_PageUp;
        case Event::CODE_KEYBOARD_PAGE_DOWN:
            return ImGuiKey_PageDown;
        case Event::CODE_KEYBOARD_HOME:
            return ImGuiKey_Home;
        case Event::CODE_KEYBOARD_END:
            return ImGuiKey_End;
        default:
            break;
    }
    DFATAL("Unknow ImGui key: %d", code);
    return ImGuiKey_Tab;
}

RendererImgui::RendererImgui(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Shader>& shader, const sp<Texture>& texture)
    : _shader(shader), _render_controller(resourceLoaderContext->renderController()), _render_engine(_render_controller->renderEngine()), _renderer_group(sp<RendererGroup>::make()), _texture(texture),
      _pipeline_factory(shader->pipelineFactory()), _renderer_context(sp<RendererContext>::make(shader, resourceLoaderContext->renderController()))
{
    _renderer_context->addDefaultTexture(texture);
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
    static const int32_t MouseIndex[5] = {0, 2, 1, 0, 0};
    ImGuiIO& io = ImGui::GetIO();
    switch(event.action())
    {
    case Event::ACTION_UP:
    case Event::ACTION_DOWN:
        io.AddMouseButtonEvent(MouseIndex[event.button() - Event::BUTTON_MOUSE_LEFT], event.action() == Event::ACTION_DOWN);
    case Event::ACTION_MOVE:
        io.AddMousePosEvent(event.x(), io.DisplaySize.y - event.y());
        break;
    case Event::ACTION_WHEEL:
        io.AddMouseWheelEvent(0, event.x());
        break;
    case Event::ACTION_KEY_DOWN:
    case Event::ACTION_KEY_UP:
        {
            const bool isKeyDown = event.action() == Event::ACTION_KEY_DOWN;
            const Event::Code code = event.code();
            switch(code)
            {
                case Event::CODE_KEYBOARD_LSHIFT:
                case Event::CODE_KEYBOARD_RSHIFT:
                case Event::CODE_KEYBOARD_LCTRL:
                case Event::CODE_KEYBOARD_RCTRL:
                case Event::CODE_KEYBOARD_LALT:
                case Event::CODE_KEYBOARD_RALT:
                case Event::CODE_KEYBOARD_TAB:
                case Event::CODE_KEYBOARD_A:
                case Event::CODE_KEYBOARD_C:
                case Event::CODE_KEYBOARD_V:
                case Event::CODE_KEYBOARD_X:
                case Event::CODE_KEYBOARD_Y:
                case Event::CODE_KEYBOARD_Z:
                case Event::CODE_KEYBOARD_LEFT:
                case Event::CODE_KEYBOARD_RIGHT:
                case Event::CODE_KEYBOARD_UP:
                case Event::CODE_KEYBOARD_DOWN:
                case Event::CODE_KEYBOARD_DELETE:
                case Event::CODE_KEYBOARD_BACKSPACE:
                case Event::CODE_KEYBOARD_ENTER:
                case Event::CODE_KEYBOARD_INSERT:
                case Event::CODE_KEYBOARD_HOME:
                case Event::CODE_KEYBOARD_END:
                case Event::CODE_KEYBOARD_PAGE_UP:
                case Event::CODE_KEYBOARD_PAGE_DOWN:
                    io.AddKeyEvent(toImGuiKey(code), isKeyDown);
                    break;
                default:
                    break;
            }
            break;
        }
    case Event::ACTION_TEXT_INPUT:
        io.AddInputCharactersUTF8(event.textInput());
        break;
    default:
        break;
    }
    return io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;
}

const sp<RendererContext>& RendererImgui::rendererContext() const
{
    return _renderer_context;
}

void RendererImgui::MyImGuiRenderFunction(const RenderRequest& renderRequest, ImDrawData* draw_data)
{
    for (int i = 0; i < draw_data->CmdListsCount; i++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[i];

        ByteArray::Borrowed vb = renderRequest.allocator().sbrkSpan(static_cast<size_t>(cmd_list->VtxBuffer.size_in_bytes()));
        ByteArray::Borrowed ib = renderRequest.allocator().sbrkSpan(static_cast<size_t>(cmd_list->IdxBuffer.size_in_bytes()));

        memcpy(vb.buf(), cmd_list->VtxBuffer.Data, static_cast<size_t>(vb.length()));
        memcpy(ib.buf(), cmd_list->IdxBuffer.Data, static_cast<size_t>(ib.length()));

        uint32_t offset = 0;
        auto ubos = _shader->takeUBOSnapshot(renderRequest);
        auto ssbos = _shader->takeSSBOSnapshot(renderRequest);
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

                const sp<DrawCommandPool>& drawCommandPool = _renderer_context->obtainDrawCommandPool(reinterpret_cast<Texture*>(pcmd->TextureId));
                sp<DrawCommandRecycler> recycler = drawCommandPool->obtainDrawCommandRecycler();
                const sp<DrawCommand>& drawCommand = recycler->drawCommand();
                Buffer::Snapshot vertexBuffer = drawCommand->_vertex_buffer.snapshot(vb);
                Buffer::Snapshot indexBuffer = drawCommand->_index_buffer.snapshot(ib);
                DrawingContext drawingContext(drawCommandPool->_shader_bindings, drawCommand->_attachments, ubos, ssbos, std::move(vertexBuffer), std::move(indexBuffer), pcmd->ElemCount, DrawingParams::DrawElements{offset});
                drawingContext._scissor = _render_engine->toRendererRect(Rect(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y, pcmd->ClipRect.z - pos.x, pcmd->ClipRect.w - pos.y), Ark::COORDINATE_SYSTEM_LHS);
                renderRequest.addRenderCommand(sp<ImguiRenderCommand>::make(drawingContext.toRenderCommand(renderRequest), std::move(recycler)));
            }
            offset += pcmd->ElemCount;
        }
    }
}

sp<RendererImgui::DrawCommandRecycler> RendererImgui::obtainDrawCommandRecycler(Texture* texture)
{
    const sp<DrawCommandPool>& drawCommandPool = _renderer_context->obtainDrawCommandPool(texture);

    sp<DrawCommand> drawCommand;
    if(!drawCommandPool->_draw_commands->pop(drawCommand))
        drawCommand = sp<DrawCommand>::make(_render_controller);

    sp<DrawCommandRecycler> recycler = sp<DrawCommandRecycler>::make(drawCommandPool->_draw_commands, drawCommand);
    return recycler;
}

RendererImgui::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _camera(sp<Camera>::make()), _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/imgui.vert", "shaders/imgui.frag", _camera))
{
}

sp<Renderer> RendererImgui::BUILDER::build(const Scope& args)
{
    const Viewport& viewport = _resource_loader_context->renderController()->renderEngine()->viewport();

    Ark::RendererCoordinateSystem coordinateSystem = _resource_loader_context->renderController()->renderEngine()->context()->coordinateSystem();
    _camera->ortho(0, viewport.width(), 0, viewport.height(), viewport.clipNear(), viewport.clipFar(), static_cast<Ark::RendererCoordinateSystem>(-coordinateSystem));

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
    sp<Bitmap> bitmap = sp<Bitmap>::make(width, height, bytesPerPixel * width, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(pixels), width * height * bytesPerPixel));
    sp<Texture> texture = _resource_loader_context->renderController()->createTexture2d(std::move(bitmap));

    return sp<RendererImgui>::make(_resource_loader_context, _shader->build(args), texture);
}

RendererImgui::DrawCommand::DrawCommand(RenderController& renderController)
    : _vertex_buffer(renderController.makeVertexBuffer()), _index_buffer(renderController.makeIndexBuffer()), _attachments(sp<ByType>::make())
{
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
