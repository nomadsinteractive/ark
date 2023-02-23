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

        operator Renderable& () const;

        sp<Renderable> _renderable;
        sp<Updatable> _updatable;

        SafeVar<Boolean> _disposed;
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

    bool preSnapshot(RenderRequest& renderRequest);
    void snapshot(RenderRequest& renderRequest, RenderLayerSnapshot& output);

    template<typename T> void doSnapshot(const T& renderables, const RenderRequest& renderRequest, RenderLayerSnapshot& output) const {
        const PipelineInput& pipelineInput = output.pipelineInput();
        const bool visible = _visible.val();
        const bool needsReload = _position_changed || _render_done != visible || output.needsReload();
        Varyings::Snapshot defaultVaryingsSnapshot = _varyings ? _varyings->snapshot(pipelineInput, renderRequest.allocator()) : Varyings::Snapshot();

        for(const auto& [i, j] : renderables) {
            Renderable& renderable = i;
            Renderable::State state = j;
            if(needsReload)
                state.setState(Renderable::RENDERABLE_STATE_DIRTY, true);
            if(state.hasState(Renderable::RENDERABLE_STATE_VISIBLE))
                state.setState(Renderable::RENDERABLE_STATE_VISIBLE, visible);
            Renderable::Snapshot snapshot = renderable.snapshot(pipelineInput, renderRequest, _position, state.stateBits());
            snapshot.applyVaryings(defaultVaryingsSnapshot);
            output.addSnapshot(*this, std::move(snapshot));
        }
    }

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
    std::vector<RenderableItem> _renderable_emplaced;
};

}

#endif
