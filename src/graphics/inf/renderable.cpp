#include "graphics/inf/renderable.h"

namespace ark {

Renderable::State::State(StateBits state)
    : _state_bits(state)
{
}

bool Renderable::State::hasState(StateBits state) const
{
    return _state_bits & state;
}

void Renderable::State::setState(StateBits state, bool enabled)
{
    _state_bits = static_cast<StateBits>((_state_bits & ~state) | (enabled ? state : 0));
}

Renderable::StateBits Renderable::State::stateBits() const
{
    return _state_bits;
}

Renderable::Snapshot::Snapshot(State state)
    : _state(state)
{
}

Renderable::Snapshot::Snapshot(State state, int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
    : _state(state), _type(type), _position(position), _size(size), _transform(transform), _varyings(varyings)
{
}

}