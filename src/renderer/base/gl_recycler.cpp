#include "renderer/base/gl_recycler.h"

#include "renderer/inf/gl_resource.h"

namespace ark {

void GLRecycler::recycle(const sp<GLResource>& resource)
{
    _items.push(resource);
}

void GLRecycler::doRecycling(GraphicsContext& graphicsContext)
{
    for(const sp<GLResource>& resource : _items)
        resource->recycle(graphicsContext);

    _items.clear();
}

}
