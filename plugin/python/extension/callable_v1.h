#pragma once

#include "core/forwarding.h"

#include "python/forwarding.h"

namespace ark::plugin::python {

class CallableV1 {
public:
    virtual ~CallableV1() = default;

//  [[script::bindings::operator(call)]]
    virtual void call(const Traits& args) = 0;
};

}