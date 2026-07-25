#include "renderer/base/recycler.h"

#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/inf/recyclable.h"

namespace ark {

void Recycler::recycle(op<Recyclable> recyclable)
{
    _recyclables.push({std::move(recyclable), 2 * kMaxFramesInFlight});
}

void Recycler::doRecycling()
{
    for(auto& [recyclable, ttl] : _recyclables.clear())
        if(--ttl > 0)
            _recyclables.push({std::move(recyclable), ttl});
}

}
