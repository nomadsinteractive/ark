#ifndef ARK_CORE_PLUGIN_SCRIPT_H_
#define ARK_CORE_PLUGIN_SCRIPT_H_

#include <list>

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API Script {
public:
    virtual ~Script() = default;

    typedef std::list<Box> Arguments;

    virtual void run(const String& script, const sp<Scope>& vars) = 0;
    virtual Box call(const String& function, const Arguments& args) = 0;

};

}

#endif
