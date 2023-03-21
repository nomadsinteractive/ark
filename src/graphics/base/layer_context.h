#pragma once

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

        operator Renderable& () const;

        sp<Renderable> _renderable;
        sp<Updatable> _updatable;

        SafeVar<Boolean> _disposed;
    };

    struct ElementState {
        element_index_t _index;
    };

public:
    LayerContext(sp<RenderBatch> batch = nullptr, sp<ModelLoader> modelLoader = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr, sp<Varyings> varyings = nullptr);

    virtual void traverse(const Visitor& visitor) override;

    SafeVar<Boolean>& visible();
    const SafeVar<Boolean>& visible() const;

    const SafeVar<Boolean>& disposed() const;

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

    bool snapshot(RenderRequest& renderRequest, RenderLayerSnapshot& output);

    bool ensureState(void* stateKey);
    ElementState& addElementState(void* key);

    bool doPreSnapshot(const RenderRequest& renderRequest, RenderLayerSnapshot& output);
    void doSnapshot(const RenderRequest& renderRequest, RenderLayerSnapshot& output);

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

    SafeVar<Boolean> _visible;
    SafeVar<Boolean> _disposed;

    sp<Varyings> _varyings;

    Layer::Type _layer_type;

    bool _reload_requested;
    bool _render_done;
    bool _position_changed;

    V3 _position;

    std::deque<std::pair<RenderableItem, Renderable::State>> _renderables;
    std::vector<RenderableItem> _renderable_created;

    std::unordered_map<void*, ElementState> _element_states;
};

}
