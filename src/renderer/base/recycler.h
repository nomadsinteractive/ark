#pragma once

#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Recycler {
public:

    void recycle(op<Recyclable> recyclable);

private:
    void doRecycling();

private:
    LFStack<op<Recyclable>> _recyclables;

    friend class ResourceManager;
    friend class RenderController;
};

}
