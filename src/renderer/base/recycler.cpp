#include "renderer/base/recycler.h"

#include "renderer/inf/resource.h"

namespace ark {

void Recycler::recycle(Resource& resource)
{
    _recyclers.push(resource.recycle());
}

void Recycler::recycle(Resource::RecycleFunc recycler)
{
    _recyclers.push(std::move(recycler));
}

void Recycler::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto& recycler : _recyclers.clear())
        recycler(graphicsContext);
}

}
