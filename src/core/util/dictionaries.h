#ifndef ARK_CORE_UTIL_DICTIONARIES_H_
#define ARK_CORE_UTIL_DICTIONARIES_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Dictionaries {
public:
    template<typename T> static T get(BeanFactory& beanFactory, const String& value, const sp<Scope>& args);

};

}

#endif
