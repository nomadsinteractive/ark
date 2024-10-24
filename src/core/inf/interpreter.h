#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API Interpreter {
public:
    virtual ~Interpreter() = default;

    typedef std::vector<Box> Arguments;

    virtual void initialize() = 0;

    virtual void execute(const sp<Asset>& source, const Scope& vars) = 0;

    virtual Box call(const Box& func, const Arguments& args) = 0;
    virtual Box attr(const Box& obj, const String& name) = 0;

};

}
