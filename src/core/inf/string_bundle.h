#pragma once

#include "core/forwarding.h"
#include "core/types/optional.h"

namespace ark {

class StringBundle {
public:
    virtual ~StringBundle() = default;

//  [[script::bindings::auto]]
    virtual Optional<String> getString(const String& resid) = 0;
//  [[script::bindings::auto]]
    virtual Vector<String> getStringArray(const String& resid) = 0;
};

}
