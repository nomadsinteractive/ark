#pragma once

#include <functional>

#include "core/base/api.h"
#include "core/types/owned_ptr.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Resource {
public:
    virtual ~Resource() = default;

    virtual uint64_t id() = 0;

    virtual void upload(GraphicsContext& graphicsContext) = 0;

    virtual op<Recyclable> toRecyclable() = 0;
};

}
