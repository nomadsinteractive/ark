#include "graphics/inf/renderable.h"

namespace ark {

Renderable::State::State(StateBits state)
    : _state_bits(state)
{
}

Renderable::State::operator bool() const
{
    return static_cast<bool>(_state_bits);
}

bool Renderable::State::hasState(StateBits state) const
{
    return _state_bits & state;
}

void Renderable::State::setState(StateBits state, bool enabled)
{
    _state_bits = static_cast<StateBits>((_state_bits & ~state) | (enabled ? state : 0));
}

void Renderable::State::setState(StateBits state)
{
    _state_bits = static_cast<StateBits>(state | (_state_bits & (RENDERABLE_STATE_NEW | RENDERABLE_STATE_DISPOSED)));
}

Renderable::StateBits Renderable::State::stateBits() const
{
    return _state_bits;
}

Renderable::Snapshot::Snapshot(State state, int32_t type)
    : _state(state), _type(type)
{
}

Renderable::Snapshot::Snapshot(State state, int32_t type, const V3& position, const V3& size, const Transform::Snapshot& transform, const Varyings::Snapshot& varyings)
    : _state(state), _type(type), _position(position), _size(size), _transform(transform), _varyings(varyings)
{
}

void Renderable::Snapshot::applyVaryings(const Varyings::Snapshot& defaultVaryingsSnapshot)
{
    if(!_varyings)
        _varyings = defaultVaryingsSnapshot;
    else
        _varyings.apply(defaultVaryingsSnapshot ? &defaultVaryingsSnapshot : nullptr);
}

}
