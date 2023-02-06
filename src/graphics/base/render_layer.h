#ifndef ARK_GRAPHICS_INF_RENDER_LAYER_H_
#define ARK_GRAPHICS_INF_RENDER_LAYER_H_

#include <deque>
#include <vector>
#include <unordered_map>

#include "core/collection/list.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"
#include "graphics/base/rect.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API RenderLayer : public Renderer {
private:
    struct Stub {
        Stub(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> disposed, sp<Varyings> varyings, sp<Vec4> scissor);

        sp<LayerContext> makeLayerContext(sp<RenderBatch> batch, sp<ModelLoader> modelLoader, sp<Boolean> visible, sp<Boolean> disposed);
        void addLayerContext(sp<LayerContext> layerContext);

        sp<RenderController> _render_controller;
        sp<ModelLoader> _model_loader;
        sp<Shader> _shader;
        sp<Vec4> _scissor;

        sp<RenderCommandComposer> _render_command_composer;
        sp<ShaderBindings> _shader_bindings;

        std::vector<sp<LayerContext>> _layer_context_list;
        sp<LayerContext> _layer_context;

        uint32_t _stride;

    };

public:
    RenderLayer(sp<RenderController> renderController, sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Boolean> visible, sp<Boolean> disposed, sp<Varyings> varyings, sp<Vec4> scissor);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    RenderLayerSnapshot snapshot(RenderRequest& renderRequest) const;

//  [[script::bindings::property]]
    const sp<LayerContext>& context() const;

//[[script::bindings::auto]]
    sp<Layer> makeLayer(sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr, sp<Vec3> position = nullptr) const;

    sp<LayerContext> makeLayerContext(sp<RenderBatch> batchOptional = nullptr, sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr, sp<Vec3> position = nullptr) const;
    void addLayerContext(sp<LayerContext> layerContext);

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<RenderLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Builder<ModelLoader>> modelLoader, sp<Builder<Shader>> shader = nullptr);

        virtual sp<RenderLayer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        std::vector<sp<Builder<Layer>>> _layers;
        sp<Builder<ModelLoader>> _model_loader;
        sp<Builder<Shader>> _shader;
        sp<Builder<Varyings>> _varyings;
        SafePtr<Builder<Boolean>> _visible;
        SafePtr<Builder<Boolean>> _disposed;
        SafePtr<Builder<Vec4>> _scissor;
    };

//  [[plugin::resource-loader("render-layer")]]
    class RENDERER_BUILDER : public Builder<Renderer> {
    public:
        RENDERER_BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    RenderLayer(const sp<Stub>& stub);

private:
    sp<Stub> _stub;

    friend class Layer;
    friend class RenderLayerSnapshot;
};

}

#endif
