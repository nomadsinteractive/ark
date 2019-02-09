#ifndef ARK_CORE_PLUGIN_SCRIPT_H_
#define ARK_CORE_PLUGIN_SCRIPT_H_

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API Script {
public:
    virtual ~Script() = default;

    typedef std::vector<Box> Arguments;

    virtual void run(const sp<Asset>& script, const sp<Scope>& vars) = 0;
    virtual Box call(const String& function, const Arguments& args) = 0;

};

}

#endif
