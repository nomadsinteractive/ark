#ifndef ARK_RENDERER_BASE_GL_RECYCLER_H_
#define ARK_RENDERER_BASE_GL_RECYCLER_H_

#include "core/base/api.h"
#include "core/concurrent/lock_free_stack.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLRecycler {
public:
    void recycle(const sp<RenderResource>& resource);

    void recycle(uint32_t id, std::function<void(uint32_t)> recycler);

    void doRecycling(GraphicsContext& graphicsContext);

private:
    LockFreeStack<sp<RenderResource>> _items;
};

}

#endif
