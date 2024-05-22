#pragma once

#include <vector>

#include "core/forwarding.h"

namespace ark {

class StringBundle {
public:
    virtual ~StringBundle() = default;

//  [[script::bindings::auto]]
    virtual sp<String> getString(const String& resid) = 0;
//  [[script::bindings::auto]]
    virtual std::vector<String> getStringArray(const String& resid) = 0;
};

}
