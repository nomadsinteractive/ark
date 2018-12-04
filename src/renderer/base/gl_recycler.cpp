#include "renderer/base/gl_recycler.h"

#include "renderer/inf/render_resource.h"

namespace ark {

void GLRecycler::recycle(RenderResource& resource)
{
    _recyclers.push(resource.recycle());
}

void GLRecycler::recycle(RenderResource::Recycler recycler)
{
    _recyclers.push(std::move(recycler));
}

void GLRecycler::doRecycling(GraphicsContext& graphicsContext)
{
    for(auto& recycler : _recyclers.clear())
        recycler(graphicsContext);
}

}
