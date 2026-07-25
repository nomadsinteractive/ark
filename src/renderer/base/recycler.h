#pragma once

#include "core/base/api.h"
#include "core/concurrent/lf_stack.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Recycler final {
public:

    void recycle(op<Recyclable> recyclable);

private:
    void doRecycling();

private:
    LFStack<std::pair<op<Recyclable>, int32_t>> _recyclables;

    friend class ResourceManager;
    friend class RenderController;
};

}
