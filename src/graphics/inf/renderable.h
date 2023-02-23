#ifndef ARK_GRAPHICS_INF_RENDERABLE_H_
#define ARK_GRAPHICS_INF_RENDERABLE_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

#include "renderer/base/varyings.h"

namespace ark {

class ARK_API Renderable {
public:
    enum StateBits {
        RENDERABLE_STATE_NONE = 0,
        RENDERABLE_STATE_DISPOSED = 1,
        RENDERABLE_STATE_DIRTY = 2,
        RENDERABLE_STATE_VISIBLE = 4
    };

    virtual ~Renderable() = default;

    class ARK_API State {
    public:
        State(StateBits state = RENDERABLE_STATE_NONE);

        explicit operator bool() const;

        bool hasState(StateBits state) const;
        void setState(StateBits state, bool enabled);

        StateBits stateBits() const;

    private:
        StateBits _state_bits;
    };

    struct Snapshot {
        Snapshot(State state = RENDERABLE_STATE_DISPOSED);
        Snapshot(State state, int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings);
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

    static StateBits toState(bool disposed, bool dirty, bool visible) {
        return static_cast<StateBits>((disposed ? RENDERABLE_STATE_DISPOSED : 0) | (dirty ? RENDERABLE_STATE_DIRTY : 0) | (visible ? RENDERABLE_STATE_VISIBLE : 0));
    }

    virtual StateBits updateState(const RenderRequest& renderRequest) = 0;
    virtual Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) = 0;
};

}

#endif
