#ifndef ARK_PLUGIN_DEAR_IMGUI_RENDERER_RENDERER_IMGUI_H_
#define ARK_PLUGIN_DEAR_IMGUI_RENDERER_RENDERER_IMGUI_H_

#include "core/inf/builder.h"
#include "core/base/memory_pool.h"
#include "core/concurrent/lf_stack.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/shader_bindings.h"

#include "app/inf/event_listener.h"

#include "dear-imgui/forwarding.h"

struct ImDrawData;
struct ImGuiIO;

namespace ark {
namespace plugin {
namespace dear_imgui {

class RendererImgui : public Renderer, public Renderer::Group, public EventListener, public Implements<RendererImgui, Renderer, Renderer::Group, EventListener> {
public:
    RendererImgui(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Shader>& shader, const sp<Texture>& texture);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;
    virtual void addRenderer(const sp<Renderer>& renderer) override;
    virtual bool onEvent(const Event& event) override;

    const sp<RendererContext>& rendererContext() const;

public:
//  [[plugin::resource-loader("imgui")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Camera> _camera;
        sp<Builder<Shader>> _shader;
    };

    struct DrawCommand {
        DrawCommand(RenderController& renderController);

        Buffer _vertex_buffer;
        Buffer _index_buffer;
        sp<ByType> _attachments;
    };

    class DrawCommandRecycler {
    public:
        DrawCommandRecycler(const sp<LFStack<sp<DrawCommand>>>& recycler, const sp<DrawCommand>& drawCommand);
        ~DrawCommandRecycler();

        const sp<DrawCommand>& drawCommand() const;

    private:
        sp<LFStack<sp<RendererImgui::DrawCommand>>> _recycler;
        sp<DrawCommand> _draw_command;
    };

private:
    void MyImGuiRenderFunction(RenderRequest& renderRequest, ImDrawData* draw_data);

    sp<DrawCommandRecycler> obtainDrawCommandRecycler(Texture* texture);

private:
    sp<Shader> _shader;
    sp<RenderController> _render_controller;
    sp<RenderEngine> _render_engine;
    sp<RendererGroup> _renderer_group;

    sp<Texture> _texture;

    MemoryPool _memory_pool;

    sp<PipelineFactory> _pipeline_factory;
    sp<RendererContext> _renderer_context;
};


}
}
}

#endif
