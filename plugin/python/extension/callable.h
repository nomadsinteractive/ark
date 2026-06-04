#pragma once

#include "core/forwarding.h"
#include "core/collection/args.h"

namespace ark::plugin::python {

class Callable {
public:
    virtual ~Callable() = default;

//  [[script::bindings::operator(call)]]
    virtual Box call(const Args& args) = 0;
};

}