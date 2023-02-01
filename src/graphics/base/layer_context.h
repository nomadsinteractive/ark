#ifndef ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_
#define ARK_GRAPHICS_BASE_LAYER_CONTEXT_H_

#include <deque>
#include <vector>

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"
#include "graphics/base/render_layer.h"
#include "graphics/inf/render_batch.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API LayerContext : public Holder {
public:
    struct RenderableItem {
        RenderableItem(sp<Renderable> renderable, sp<Updatable> updatable, sp<Boolean> disposed);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RenderableItem);

        sp<Renderable> _renderable;
        sp<Updatable> _updatable;

        SafeVar<Boolean> _disposed;

        Renderable::State _state;
    };

public:
    LayerContext(sp<RenderBatch> batch = nullptr, sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr, sp<Varyings> varyings = nullptr);

    virtual void traverse(const Visitor& visitor) override;

    SafeVar<Visibility>& visible();
    const SafeVar<Visibility>& visible() const;

    const SafeVar<Disposed>& disposed() const;

    bool isDisposed() const;

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;
    void setModelLoader(sp<ModelLoader> modelLoader);

    Layer::Type layerType() const;

    void renderRequest(const V3& position);

//  [[script::bindings::auto]]
    void add(sp<Renderable> renderable, sp<Updatable> isDirty = nullptr, sp<Boolean> isDisposed = nullptr);
//  [[script::bindings::auto]]
    void clear();

//  [[script::bindings::property]]
    const sp<Varyings>& varyings() const;
//  [[script::bindings::property]]
    void setVaryings(sp<Varyings> varyings);

    const std::deque<RenderableItem>& instances() const;

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

public:
    sp<RenderBatch> _render_batch;

    sp<ModelLoader> _model_loader;
    SafeVar<Visibility> _visible;
    SafeVar<Disposed> _disposed;
    sp<Varyings> _varyings;

    Layer::Type _layer_type;

    bool _reload_requested;
    bool _render_done;
    bool _position_changed;

    V3 _position;

    std::deque<RenderableItem> _renderables;
    std::vector<RenderableItem> _renderable_emplaced;
};

}

#endif
