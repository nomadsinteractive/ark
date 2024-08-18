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
        RENDERABLE_STATE_NEW = 2,
        RENDERABLE_STATE_DIRTY = 4,
        RENDERABLE_STATE_VISIBLE = 8,
        RENDERABLE_STATE_OCCLUSION_QUERY = 16
    };

    virtual ~Renderable() = default;

    class ARK_API State {
    public:
        State(StateBits state = RENDERABLE_STATE_NONE);

        explicit operator bool() const;

        bool has(StateBits state) const;
        void set(StateBits state, bool enabled);
        void reset(StateBits state);

        StateBits stateBits() const;

    private:
        StateBits _state_bits;
    };

    struct Snapshot {

        void applyVaryings(const Varyings::Snapshot& defaultVaryingsSnapshot);

        State _state = RENDERABLE_STATE_DISCARDED;
        int32_t _type = 0;
        sp<Model> _model = nullptr;
        V3 _position{0};
        V3 _size{1.0f};
        Transform::Snapshot _transform;
        Varyings::Snapshot _varyings;
    };

    virtual StateBits updateState(const RenderRequest& renderRequest) = 0;
    virtual Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, StateBits state) = 0;
};

}
