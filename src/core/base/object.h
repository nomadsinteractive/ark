#ifndef ARK_CORE_BASE_OBJECT_H_
#define ARK_CORE_BASE_OBJECT_H_

#include "core/base/api.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Object {
public:
    virtual ~Object() = default;
};

}

#endif
