#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API Interpreter {
public:
    virtual ~Interpreter() = default;

    typedef Vector<Box> Arguments;

    virtual void initialize() = 0;

    virtual void execute(const sp<Asset>& source) = 0;

    virtual Box call(const Box& func, const Arguments& args) = 0;
    virtual Box attr(const Box& obj, StringView name) = 0;
};

}
