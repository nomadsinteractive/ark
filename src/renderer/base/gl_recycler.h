#ifndef ARK_RENDERER_BASE_GL_RECYCLER_H_
#define ARK_RENDERER_BASE_GL_RECYCLER_H_

#include "core/base/api.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_resource.h"

namespace ark {

class ARK_API GLRecycler {
public:

    void recycle(RenderResource& resource);
    void recycle(RenderResource::Recycler recycler);

    void doRecycling(GraphicsContext& graphicsContext);

private:
    LockFreeStack<RenderResource::Recycler> _recyclers;
};

}

#endif
