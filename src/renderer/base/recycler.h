#pragma once

#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"

#include "renderer/forwarding.h"

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
