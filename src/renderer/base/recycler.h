#ifndef ARK_RENDERER_BASE_RECYCLER_H_
#define ARK_RENDERER_BASE_RECYCLER_H_

#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/resource.h"

namespace ark {

class ARK_API Recycler {
public:

    void recycle(Resource& resource);
    void recycle(ResourceRecycleFunc recycler);

private:
    void doRecycling(GraphicsContext& graphicsContext);

private:
    LFStack<ResourceRecycleFunc> _recyclers;

    friend class ResourceManager;
    friend class RenderController;
};

}

#endif
