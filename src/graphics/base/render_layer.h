#pragma once

#include <vector>

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

        sp<RenderCommandComposer> _render_command_composer;
        sp<PipelineBindings> _pipeline_bindings;

        uint32_t _stride;
    };

public:
    RenderLayer(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> discarded, sp<Varyings> varyings, sp<Vec4> scissor);

    void render(RenderRequest& renderRequest, const V3& position) override;

    RenderLayerSnapshot snapshot(RenderRequest& renderRequest);

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;

//  [[script::bindings::auto]]
    sp<Layer> makeLayer(sp<ModelLoader> modelLoader = nullptr, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr) const;

    void dispose();

    sp<LayerContext> makeLayerContext(sp<ModelLoader> modelLoader, sp<Vec3> position, sp<Boolean> visible, sp<Boolean> discarded) const;
    sp<LayerContext> addLayerContext(sp<ModelLoader> modelLoader = nullptr, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr) const;
    void addLayerContext(sp<LayerContext> layerContext);

    void addRenderBatch(sp<RenderBatch> renderBatch);

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<RenderLayer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        std::vector<sp<Builder<Layer>>> _layers;
        sp<Builder<ModelLoader>> _model_loader;
        sp<Builder<Shader>> _shader;
        sp<Builder<Varyings>> _varyings;
        SafeBuilder<Boolean> _visible;
        SafeBuilder<Boolean> _discarded;
        SafeBuilder<Vec4> _scissor;
        bool _post_process;
    };

//  [[plugin::resource-loader("render-layer")]]
    class RENDERER_BUILDER : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    RenderLayer(sp<Stub> stub);

private:
    sp<Stub> _stub;

    class RenderBatchImpl;
    sp<RenderBatchImpl> _render_batch;
    std::vector<sp<RenderBatch>> _render_batches;

    friend class Layer;
    friend class RenderLayerSnapshot;
    friend class LayerContextSnapshot;
};

}
