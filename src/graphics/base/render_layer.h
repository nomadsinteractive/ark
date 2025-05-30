#pragma once

#include "core/collection/list.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/components/layer.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API RenderLayer final : public Renderer {
private:
    struct Stub {
        Stub(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor);

        sp<RenderController> _render_controller;
        sp<ModelLoader> _model_loader;
        sp<Shader> _shader;
        SafeVar<Boolean> _visible;
        SafeVar<Boolean> _discarded;
        sp<Varyings> _varyings;
        sp<Vec4> _scissor;

        sp<DrawingContextComposer> _drawing_context_composer;
        sp<PipelineBindings> _pipeline_bindings;

        uint32_t _stride;
    };

public:
    RenderLayer(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor);

    void render(RenderRequest& renderRequest, const V3& position, const sp<DrawDecorator>& drawDecorator) override;

    sp<RenderCommand> compose(const RenderRequest& renderRequest, sp<DrawDecorator> drawDecorator = nullptr);

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;

//  [[script::bindings::auto]]
    sp<Layer> makeLayer(sp<ModelLoader> modelLoader = nullptr, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr) const;

    sp<LayerContext> makeLayerContext(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded) const;
    sp<LayerContext> addLayerContext(sp<ModelLoader> modelLoader = nullptr, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr) const;
    void addLayerContext(sp<LayerContext> layerContext);

    void addRenderBatch(sp<RenderBatch> renderBatch);

//  [[plugin::builder]]
    class BUILDER final : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);
        BUILDER(BeanFactory& factory, const document& manifest, sp<Builder<ModelLoader>> modelLoader);

        sp<RenderLayer> build(const Scope& args) override;

    private:
        sp<Builder<ModelLoader>> _model_loader;
        sp<Builder<Shader>> _shader;
        sp<Builder<Varyings>> _varyings;
        SafeBuilder<Boolean> _visible;
        SafeBuilder<Boolean> _discarded;
        SafeBuilder<Vec4> _scissor;
    };

//  [[plugin::builder("render-layer")]]
    class RENDERER_BUILDER final : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest);

        sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

//  [[plugin::builder("post-process")]]
    class RENDERER_POST_PROCESS final : public Builder<Renderer> {
    public:
        RENDERER_POST_PROCESS(BeanFactory& factory, const document& manifest);

        sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    RenderLayer(sp<Stub> stub);

    RenderLayerSnapshot snapshot(const RenderRequest& renderRequest);

private:
    sp<Stub> _stub;

    class RenderBatchImpl;
    sp<RenderBatchImpl> _render_batch;
    Vector<sp<RenderBatch>> _render_batches;

    friend class Layer;
    friend class RenderLayerSnapshot;
    friend class RenderTarget;
    friend class LayerContextSnapshot;
};

}
