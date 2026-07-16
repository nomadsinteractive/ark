#pragma once

#include <deque>
#include <limits>

#include "core/base/api.h"
#include "core/base/timestamp.h"

#include "graphics/forwarding.h"
#include "graphics/base/render_layer.h"
#include "graphics/inf/renderable.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API LayerContext final  {
public:
    struct ElementState {
        Optional<element_index_t> _index;
    };

    struct FrameState {
        struct RenderableState {
            Renderable* _renderable;
            ElementState* _element_state;
            Renderable::State _state;
        };

        uint32_t _tick = std::numeric_limits<uint32_t>::max();
        bool _dirty = false;
        bool _instances_dirty = false;
        Vector<RenderableState> _renderable_states;
        Vector<ElementState> _discarded_element_states;
    };

public:
    LayerContext(sp<Shader> shader = nullptr, sp<ModelLoader> modelLoader = nullptr, sp<Vec3> position = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr, sp<Varyings> varyings = nullptr, sp<Updatable> updatable = nullptr);

    const sp<Shader>& shader() const;

    const OptionalVar<Vec3>& position() const;
    void setPosition(sp<Vec3> position);

    OptionalVar<Boolean>& visible();
    const OptionalVar<Boolean>& visible() const;

    const OptionalVar<Boolean>& discarded() const;

    const sp<ModelLoader>& modelLoader() const;
    void setModelLoader(sp<ModelLoader> modelLoader);

    void pushBack(sp<Renderable> renderable);
    void clear();
    void discard();

    void markDirty();

    LayerContextSnapshot snapshot(const RenderRequest& renderRequest, const PipelineLayout& pipelineLayout);

private:
    void updateFrameState(const RenderRequest& renderRequest);
    bool processNewCreated();
    ElementState& addElementState(void* key);

private:
    sp<Shader> _shader;
    sp<ModelLoader> _model_loader;

    OptionalVar<Vec3> _position;
    OptionalVar<Boolean> _visible;
    OptionalVar<Boolean> _discarded;

    sp<Varyings> _varyings;
    sp<Updatable> _updatable;

    std::deque<std::pair<sp<Renderable>, Renderable::State>> _renderables;
    Vector<sp<Renderable>> _newly_created_renderables;

    HashMap<const void*, ElementState> _element_states;
    FrameState _frame_state;

    Timestamp _timestamp;

    friend class RenderLayerSnapshot;
};

}
