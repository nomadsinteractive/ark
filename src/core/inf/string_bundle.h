#ifndef ARK_CORE_INF_STRING_BUNDLE_H_
#define ARK_CORE_INF_STRING_BUNDLE_H_

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

#endif
