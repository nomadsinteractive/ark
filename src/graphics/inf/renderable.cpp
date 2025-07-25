#include "graphics/inf/renderable.h"

namespace ark {

void Renderable::Snapshot::applyVaryings(const Varyings::Snapshot& defaultVaryingsSnapshot)
{
    if(!_varyings_snapshot)
        _varyings_snapshot._buffers = defaultVaryingsSnapshot._buffers;
    else
        _varyings_snapshot.apply(defaultVaryingsSnapshot ? &defaultVaryingsSnapshot : nullptr);
}

}
