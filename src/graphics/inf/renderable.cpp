#include "graphics/inf/renderable.h"

namespace ark {

// Renderable::State::State(StateBits state)
//     : _state_bits(state)
// {
// }
//
// Renderable::State::operator bool() const
// {
//     return static_cast<bool>(_state_bits);
// }
//
// bool Renderable::State::has(StateBits state) const
// {
//     return _state_bits & state;
// }
//
// void Renderable::State::set(StateBits state, bool enabled)
// {
//     _state_bits = static_cast<StateBits>((_state_bits & ~state) | (enabled ? state : 0));
// }
//
// void Renderable::State::reset(StateBits state)
// {
//     _state_bits = static_cast<StateBits>(state | (_state_bits & (RENDERABLE_STATE_NEW | RENDERABLE_STATE_DISCARDED)));
// }
//
// Renderable::StateBits Renderable::State::stateBits() const
// {
//     return _state_bits;
// }

void Renderable::Snapshot::applyVaryings(const Varyings::Snapshot& defaultVaryingsSnapshot)
{
    if(!_varyings_snapshot)
        _varyings_snapshot._buffers = defaultVaryingsSnapshot._buffers;
    else
        _varyings_snapshot.apply(defaultVaryingsSnapshot ? &defaultVaryingsSnapshot : nullptr);
}

}
