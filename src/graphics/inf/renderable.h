#pragma once

#include "core/base/api.h"
#include "core/base/bit_set.h"

#include "graphics/forwarding.h"
#include "graphics/inf/transform.h"
#include "graphics/base/v3.h"

#include "renderer/components/varyings.h"

namespace ark {

class ARK_API Renderable {
public:
    virtual ~Renderable() = default;

    enum StateBits {
        RENDERABLE_STATE_NONE = 0,
        RENDERABLE_STATE_DISCARDED = 1,
        RENDERABLE_STATE_NEW = 2,
        RENDERABLE_STATE_DIRTY = 4,
        RENDERABLE_STATE_VISIBLE = 8,
        RENDERABLE_STATE_OCCLUSION_QUERY = 16
    };
    typedef BitSet<StateBits> State;

    struct Snapshot {

        void applyVaryings(const Varyings::Snapshot& defaultVaryingsSnapshot);

        State _state = {RENDERABLE_STATE_DISCARDED};
        int32_t _type = 0;
        sp<Model> _model = nullptr;
        V3 _position{0};
        V3 _size{1.0f};
        sp<Transform> _transform = nullptr;
        Transform::Snapshot _transform_snapshot;
        Varyings::Snapshot _varyings_snapshot;
    };

    virtual State updateState(const RenderRequest& renderRequest) = 0;
    virtual Snapshot snapshot(const LayerContextSnapshot& snapshotContext, const RenderRequest& renderRequest, State state) = 0;
};

}
