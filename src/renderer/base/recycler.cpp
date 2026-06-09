#include "renderer/base/recycler.h"

#include "core/types/owned_ptr.h"

#include "renderer/inf/recyclable.h"

namespace ark {

void Recycler::recycle(op<Recyclable> recyclable)
{
    _recyclables.push(std::move(recyclable));
}

void Recycler::doRecycling()
{
    // Recyclables clean up in their destructors; clearing the stack drops the owning ptrs and frees the resources.
    _recyclables.clear();
}

}
