#pragma once

#include "core/inf/builder.h"
#include "core/concurrent/lf_stack.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/pipeline_bindings.h"

#include "app/inf/event_listener.h"

#include "dear-imgui/forwarding.h"

struct ImDrawData;
struct ImGuiIO;

namespace ark::plugin::dear_imgui {

class RendererImgui : public Renderer, public Renderer::Group, public EventListener, public Implements<RendererImgui, Renderer, Renderer::Group, EventListener> {
public:
    RendererImgui(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Shader>& shader, const sp<Texture>& texture);

    void render(RenderRequest& renderRequest, const V3& position) override;
    void addRenderer(sp<Renderer> renderer, const Traits& traits) override;
    bool onEvent(const Event& event) override;

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
        sp<Traits> _attachments;
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
    void MyImGuiRenderFunction(const RenderRequest& renderRequest, ImDrawData* draw_data);

    sp<DrawCommandRecycler> obtainDrawCommandRecycler(Texture* texture);

private:
    sp<Shader> _shader;
    sp<RenderController> _render_controller;
    sp<RenderEngine> _render_engine;
    sp<RendererGroup> _renderer_group;

    sp<Texture> _texture;

    sp<PipelineFactory> _pipeline_factory;
    sp<RendererContext> _renderer_context;
};

}
