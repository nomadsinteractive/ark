#pragma once

#include "core/base/api.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Recyclable {
public:
    virtual ~Recyclable() = default;
};

}
