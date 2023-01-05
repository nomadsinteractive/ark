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
        Stub(sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Vec4> scissor, sp<RenderController> renderController);

        sp<LayerContext> makeLayerContext(sp<RenderableBatch> batch, sp<ModelLoader> modelLoader, sp<Boolean> visible, sp<Boolean> disposed);

        sp<ModelLoader> _model_loader;
        sp<Shader> _shader;
        sp<Vec4> _scissor;

        sp<RenderController> _render_controller;
        sp<RenderCommandComposer> _render_command_composer;
        sp<ShaderBindings> _shader_bindings;

        SafeVar<Visibility> _visible;
        std::vector<sp<LayerContext>> _batch_groups;
        sp<Layer> _layer;

        uint32_t _stride;

    };

public:
    RenderLayer(sp<ModelLoader> modelLoader, sp<Shader> shader, sp<Vec4> scissor, sp<RenderController> renderController);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    RenderLayerSnapshot snapshot(RenderRequest& renderRequest) const;

//  [[script::bindings::property]]
    const sp<Layer>& layer() const;
//  [[script::bindings::property]]
    const sp<LayerContext>& context() const;

//[[script::bindings::auto]]
    sp<Layer> makeLayer(sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr) const;

    sp<LayerContext> makeContext(sp<RenderableBatch> batch = nullptr, sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr) const;

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
