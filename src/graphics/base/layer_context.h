#pragma once

#include <deque>
#include <vector>

#include "core/base/api.h"
#include "core/base/timestamp.h"
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
class ARK_API LayerContext : public Updatable, public Holder {
public:
    struct ElementState {
        Optional<element_index_t> _index;
    };

    struct Snapshot {
        bool _dirty;
        V3 _position;
        bool _visible;
        bool _disposed;
        Varyings::Snapshot _varyings;
    };

public:
    LayerContext(sp<ModelLoader> modelLoader = nullptr, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr, sp<Varyings> varyings = nullptr);

    virtual bool update(uint64_t timestamp) override;
    virtual void traverse(const Visitor& visitor) override;

    const SafeVar<Vec3>& position() const;
    void setPosition(sp<Vec3> position);

    SafeVar<Boolean>& visible();
    const SafeVar<Boolean>& visible() const;

    const SafeVar<Boolean>& disposed() const;

//  [[script::bindings::property]]
    const sp<ModelLoader>& modelLoader() const;
    void setModelLoader(sp<ModelLoader> modelLoader);

    Layer::Type layerType() const;

//  [[script::bindings::auto]]
    void add(sp<Renderable> renderable, sp<Updatable> isDirty = nullptr, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    void clear();
    void dispose();
//  [[script::bindings::property]]
    const sp<Varyings>& varyings() const;
//  [[script::bindings::property]]
    void setVaryings(sp<Varyings> varyings);

    void markDirty();

    bool processNewCreated();

    Snapshot snapshot(RenderRequest& renderRequest, const PipelineInput& pipelineInput) const;

    bool ensureState(void* stateKey);
    ElementState& addElementState(void* key);

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
    sp<ModelLoader> _model_loader;

    SafeVar<Vec3> _position;
    SafeVar<Boolean> _visible;
    SafeVar<Boolean> _discarded;

    sp<Varyings> _varyings;

    Layer::Type _layer_type;

    bool _reload_requested;

    std::deque<std::pair<sp<Renderable>, Renderable::State>> _renderables;
    std::vector<sp<Renderable>> _renderable_created;

    std::unordered_map<void*, ElementState> _element_states;

    Timestamp _timestamp;

};

}
