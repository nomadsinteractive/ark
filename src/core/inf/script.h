#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API Script {
public:
    virtual ~Script() = default;

    typedef std::vector<Box> Arguments;

    virtual void run(const sp<Asset>& script, const Scope& vars) = 0;
    virtual Box call(const String& function, const Arguments& args) = 0;

};

}
