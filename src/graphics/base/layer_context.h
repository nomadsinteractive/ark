#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include <deque>

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/layer.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderable_batch.h"

#include "renderer/forwarding.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API LayerContext : public Holder {
public:
    struct Instance {
        Instance(sp<Renderable> renderable, sp<Boolean> disposed);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Instance);

        sp<Renderable> _renderable;
        SafeVar<Boolean> _disposed;
        Renderable::State _state;
    };

public:
    LayerContext(sp<RenderableBatch> batch, sp<ModelLoader> modelLoader, sp<Boolean> visible, sp<Boolean> disposed, sp<Varyings> varyings);

    virtual void traverse(const Visitor& visitor) override;

    SafeVar<Visibility>& visible();
    const SafeVar<Visibility>& visible() const;

    const SafeVar<Disposed>& disposed() const;

    bool isDisposed() const;

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;

    Layer::Type layerType() const;

    void renderRequest(const V3& position);

    void add(sp<Renderable> renderable, sp<Boolean> disposed = nullptr);
//  [[script::bindings::auto]]
    void addRenderObject(const sp<RenderObject>& renderObject, const sp<Boolean>& disposed = nullptr);
//  [[script::bindings::auto]]
    void clear();

//  [[script::bindings::property]]
    const sp<Varyings>& varyings() const;
//  [[script::bindings::property]]
    void setVaryings(sp<Varyings> varyings);

    const std::deque<Instance>& instances() const;

    bool preSnapshot(RenderRequest& renderRequest);
    void snapshot(RenderRequest& renderRequest, RenderLayerSnapshot& output);

    class BUILDER : public Builder<LayerContext> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, Layer::Type layerType);

        virtual sp<LayerContext> build(const Scope& args) override;

    private:
        sp<Builder<Layer>> _layer;
        sp<Builder<RenderLayer>> _render_layer;
        Layer::Type _layer_type;
    };

private:
    class DefaultBatch : public RenderableBatch {
    public:
        virtual bool preSnapshot(const RenderRequest& renderRequest, LayerContext& lc) override;
        virtual void snapshot(const RenderRequest& renderRequest, const LayerContext& lc, RenderLayerSnapshot& output) override;
    };

public:
    sp<RenderableBatch> _batch;

    sp<ModelLoader> _model_loader;
    SafeVar<Visibility> _visible;
    SafeVar<Disposed> _disposed;
    sp<Varyings> _varyings;

    Layer::Type _layer_type;

    bool _reload_requested;
    bool _render_done;
    bool _position_changed;

    V3 _position;

    std::deque<Instance> _renderables;
    std::vector<Instance> _renderable_emplaced;
};

}

#endif
