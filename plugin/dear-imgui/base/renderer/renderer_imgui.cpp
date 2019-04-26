#include "dear-imgui/base/renderer/renderer_imgui.h"

#include <imgui.h>

#include "core/ark.h"
#include "core/types/global.h"
#include "core/inf/asset.h"
#include "core/inf/readable.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/camera.h"
#include "graphics/base/render_request.h"
#include "graphics/inf/render_command.h"
#include "graphics/impl/renderer/renderer_group.h"

#include "renderer/base/drawing_context.h"
#include "renderer/base/pipeline_input.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/base/render_engine.h"
#include "renderer/inf/render_model.h"
#include "renderer/inf/uploader.h"
#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/event.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

namespace {

class ImguiContext {
public:
    ImguiContext()
        : _context(ImGui::CreateContext()) {
    }
    ~ImguiContext() {
        ImGui::DestroyContext(_context);
    }

private:
    ImGuiContext* _context;
};


class ImguiRenderCommand : public RenderCommand {
public:
    ImguiRenderCommand(sp<RenderCommand> delegate, sp<RendererImgui::DrawCommand> drawCommand)
        : _delegate(std::move(delegate)), _draw_command(std::move(drawCommand)) {
    }

    virtual void draw(GraphicsContext& graphicsContext) override {
        _delegate->draw(graphicsContext);
        _draw_command->recycle(_draw_command);
    }

private:
    sp<RenderCommand> _delegate;
    sp<RendererImgui::DrawCommand> _draw_command;
};

}

RendererImgui::RendererImgui(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Shader>& shader, const sp<Texture>& texture)
    : _shader(shader), _render_controller(resourceLoaderContext->renderController()), _render_engine(_render_controller->renderEngine()), _renderer_group(sp<RendererGroup>::make()), _texture(texture),
      _draw_commands(sp<LFStack<sp<DrawCommand>>>::make())
{
}

void RendererImgui::render(RenderRequest& renderRequest, float x, float y)
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = _render_engine->viewport().width();
    io.DisplaySize.y = _render_engine->viewport().height();

    ImGui::NewFrame();
    _renderer_group->render(renderRequest, x, y);
    ImGui::EndFrame();
    ImGui::Render();
    MyImGuiRenderFunction(renderRequest, io, ImGui::GetDrawData());
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
    default:
        break;
    }
    return false;
}

sp<RendererImgui::DrawCommand> RendererImgui::obtainDrawCommand()
{
    sp<DrawCommand> cmd;
    if(_draw_commands->pop(cmd))
        return cmd;

    return sp<DrawCommand>::make(_shader, _render_controller, _texture, _draw_commands);
}

void RendererImgui::MyImGuiRenderFunction(RenderRequest& renderRequest, ImGuiIO& io, ImDrawData* draw_data)
{
    for (int i = 0; i < draw_data->CmdListsCount; i++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[i];

        bytearray vb = _memory_pool.allocate(static_cast<size_t>(cmd_list->VtxBuffer.size_in_bytes()));
        bytearray ib = _memory_pool.allocate(static_cast<size_t>(cmd_list->IdxBuffer.size_in_bytes()));

        memcpy(vb->buf(), cmd_list->VtxBuffer.Data, static_cast<size_t>(cmd_list->VtxBuffer.size_in_bytes()));
        memcpy(ib->buf(), cmd_list->IdxBuffer.Data, static_cast<size_t>(cmd_list->IdxBuffer.size_in_bytes()));

        sp<DrawCommand> drawCommand = obtainDrawCommand();
        Buffer::Snapshot vertexBuffer = drawCommand->_vertex_buffer.snapshot(_object_pool.obtain<Uploader::Array<uint8_t>>(vb));
        Buffer::Snapshot indexBuffer = drawCommand->_index_buffer.snapshot(_object_pool.obtain<Uploader::Array<uint8_t>>(ib));

        uint32_t offset = 0;
        const std::vector<RenderLayer::UBOSnapshot> ubos = _shader->snapshot(_memory_pool);

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
                DrawingContext drawingContext(_shader, drawCommand->_shader_bindings, ubos, vertexBuffer, indexBuffer, static_cast<int32_t>(pcmd->ElemCount / 3), offset, offset + pcmd->ElemCount);
                drawingContext._parameters._cull_face = false;
                drawingContext._parameters._scissor = Rect(pcmd->ClipRect.x - pos.x, io.DisplaySize.y - pcmd->ClipRect.y + pos.y, pcmd->ClipRect.z - pos.x, io.DisplaySize.y - pcmd->ClipRect.w + pos.y);
                renderRequest.addRequest(sp<ImguiRenderCommand>::make(drawingContext.toRenderCommand(_object_pool), drawCommand));
            }
            offset += pcmd->ElemCount;
        }
    }
}

RendererImgui::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _camera(sp<Camera>::make()), _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/imgui.vert", "shaders/imgui.frag", _camera)),
      _font(manifest->getChild("font"))
{
}

sp<Renderer> RendererImgui::BUILDER::build(const sp<Scope>& args)
{
    const Viewport& viewport = _resource_loader_context->renderController()->renderEngine()->viewport();
    _camera->ortho(0, viewport.width(), viewport.height(), 0, viewport.near(), viewport.far());

    const Global<ImguiContext> context;
    ImGuiIO& io = ImGui::GetIO();

    if(_font)
    {
        ImFontConfig fontConfig;
        const String& src = Documents::getAttribute(_font, Constants::Attributes::SRC);
        fontConfig.SizePixels = Documents::getAttribute<float>(_font, Constants::Attributes::SIZE, 18.0f);
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
            io.Fonts->AddFontFromMemoryTTF(data, datasize, fontConfig.SizePixels, &fontConfig);
        }
    }

    unsigned char* pixels = nullptr;
    int32_t width, height, bytesPerPixel;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel);
    const sp<Bitmap> bitmap = sp<Bitmap>::make(width, height, bytesPerPixel * width, 4, sp<ByteArray::Borrowed>::make(reinterpret_cast<uint8_t*>(pixels), width * height * bytesPerPixel));
    const sp<Texture> texture = _resource_loader_context->renderController()->createTexture(static_cast<uint32_t>(width), static_cast<uint32_t>(height), sp<Texture::UploaderBitmap>::make(bitmap));

    return sp<RendererImgui>::make(_resource_loader_context, _shader->build(args), texture);
}

RendererImgui::DrawCommand::DrawCommand(Shader& shader, RenderController& renderController, const sp<Texture>& texture, const sp<LFStack<sp<DrawCommand>>>& recycler)
    : _vertex_buffer(renderController.makeVertexBuffer()), _index_buffer(renderController.makeIndexBuffer()),
      _shader_bindings(sp<ShaderBindings>::make(RenderModel::RENDER_MODE_TRIANGLES, renderController, shader.pipelineLayout(), _vertex_buffer, _index_buffer)),
      _recycler(recycler)
{
    _shader_bindings->bindSampler(texture);
}

void RendererImgui::DrawCommand::recycle(const sp<RendererImgui::DrawCommand>& self)
{
    _recycler->push(self);
}

}
}
}
