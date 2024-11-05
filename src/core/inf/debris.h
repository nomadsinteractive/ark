#pragma once

#include <functional>

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API Debris {
public:
    virtual ~Debris() = default;

    typedef std::function<void(const Box&)> Visitor;

    virtual void traverse(const Visitor& visitor) = 0;
};

}
