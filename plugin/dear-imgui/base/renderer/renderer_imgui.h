#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_RENDERER_IMGUI_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_RENDERER_IMGUI_H_

#include "core/inf/builder.h"
#include "core/base/memory_pool.h"
#include "core/base/object_pool.h"
#include "core/concurrent/lf_stack.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/shader_bindings.h"

#include "app/inf/event_listener.h"

struct ImDrawData;
struct ImGuiIO;

namespace ark {
namespace plugin {
namespace dear_imgui {

class RendererImgui : public Renderer, public Renderer::Group, public EventListener, public Implements<RendererImgui, Renderer, Renderer::Group, EventListener> {
public:
    RendererImgui(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Shader>& shader, const sp<Texture>& texture);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;
    virtual void addRenderer(const sp<Renderer>& renderer) override;
    virtual bool onEvent(const Event& event) override;

public:
//  [[plugin::resource-loader("imgui")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Camera> _camera;
        sp<Builder<Shader>> _shader;
    };

    struct DrawCommand {
        DrawCommand(const Shader& shader, const sp<PipelineFactory>& pipelineFactory, RenderController& renderController, const sp<Texture>& texture, const sp<LFStack<sp<DrawCommand>>>& recycler);

        Buffer _vertex_buffer;
        Buffer _index_buffer;
        sp<ShaderBindings> _shader_bindings;

        void recycle(const sp<DrawCommand>& self);

        sp<LFStack<sp<DrawCommand>>> _recycler;
    };

    sp<DrawCommand> obtainDrawCommand();

private:
    void MyImGuiRenderFunction(RenderRequest& renderRequest, ImDrawData* draw_data);

private:
    sp<Shader> _shader;
    sp<RenderController> _render_controller;
    sp<RenderEngine> _render_engine;
    sp<RendererGroup> _renderer_group;

    sp<Texture> _texture;

    MemoryPool _memory_pool;
    ObjectPool _object_pool;

    sp<LFStack<sp<DrawCommand>>> _draw_commands;
    sp<PipelineFactory> _bindings;

    bool _vflip;
    V2 _viewport_scale;
};


}
}
}

#endif
