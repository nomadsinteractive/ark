#ifndef ARK_GRAPHICS_INF_RENDERABLE_H_
#define ARK_GRAPHICS_INF_RENDERABLE_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

#include "renderer/base/varyings.h"

namespace ark {

class ARK_API Renderable {
public:
    enum State {
        RENDERABLE_STATE_NORMAL = 0,
        RENDERABLE_STATE_DISPOSED = 1,
        RENDERABLE_STATE_DIRTY = 2,
        RENDERABLE_STATE_VISIBLE = 4
    };

    virtual ~Renderable() = default;

    struct Snapshot {
        Snapshot(State state = RENDERABLE_STATE_DISPOSED)
            : _state(state) {
        }
        Snapshot(State state, int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
            : _state(state), _type(type), _position(position),
              _size(size), _transform(transform), _varyings(varyings) {
        }
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Snapshot);

        bool getState(State state) const {
            return _state & state;
        }

        void setState(State state, bool enabled) {
            _state = static_cast<State>((_state & ~state) | (enabled ? state : 0));
        }

        State _state;
        int32_t _type;
        sp<Model> _model;
        V3 _position;
        V3 _size;
        Transform::Snapshot _transform;
        Varyings::Snapshot _varyings;
    };

    static State toState(bool disposed, bool dirty, bool visible) {
        return static_cast<State>((disposed ? RENDERABLE_STATE_DISPOSED : 0) | (dirty ? RENDERABLE_STATE_DIRTY : 0) | (visible ? RENDERABLE_STATE_VISIBLE : 0));
    }

    static void setState(State& s0, State s1, bool enabled) {
        s0 = static_cast<State>(enabled ? s0 | s1 : s0 & ~s1);
    }

    virtual State updateState(const RenderRequest& renderRequest) = 0;
    virtual Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, State state) = 0;
};

}

#endif
