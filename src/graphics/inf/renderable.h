#pragma once

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

#include "renderer/base/varyings.h"

namespace ark {

class ARK_API Renderable {
public:
    enum StateBits {
        RENDERABLE_STATE_NONE = 0,
        RENDERABLE_STATE_DISCARDED = 1,
        RENDERABLE_STATE_DIRTY = 2,
        RENDERABLE_STATE_VISIBLE = 4,
        RENDERABLE_STATE_NEW = 8
    };

    virtual ~Renderable() = default;

    class ARK_API State {
    public:
        State(StateBits state = RENDERABLE_STATE_NONE);

        explicit operator bool() const;

        bool hasState(StateBits state) const;
        void setState(StateBits state, bool enabled);
        void setState(StateBits state);

        StateBits stateBits() const;

    private:
        StateBits _state_bits;
    };

    struct Snapshot {
        Snapshot(State state = RENDERABLE_STATE_DISCARDED, int32_t type = 0, sp<Model> model = nullptr);
        Snapshot(State state, int32_t type, sp<Model> model, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        void applyVaryings(const Varyings::Snapshot& defaultVaryingsSnapshot);

        State _state;
        int32_t _type;
        sp<Model> _model;
        V3 _position;
        V3 _size;
        Transform::Snapshot _transform;
        Varyings::Snapshot _varyings;
    };

    virtual StateBits updateState(const RenderRequest& renderRequest) = 0;
    virtual Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) = 0;
};

}
