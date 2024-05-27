#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Block {
public:
    virtual ~Block() = default;

    virtual const sp<Size>& size() = 0;
};

}
