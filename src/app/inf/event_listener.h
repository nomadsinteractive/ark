#pragma once

#include "core/base/api.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API EventListener {
public:
    virtual ~EventListener() = default;

    virtual bool onEvent(const Event& event) = 0;
};

}
