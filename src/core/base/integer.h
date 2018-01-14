#ifndef ARK_CORE_BASE_INTEGER_H_
#define ARK_CORE_BASE_INTEGER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Integer : public Variable<int32_t> {
public:
    virtual ~Integer() = default;

//[[script::bindings::constructor]]
    static sp<Integer> __init__(const sp<Integer>& value);
//[[script::bindings::constructor]]
    static sp<Integer> __init__(const int32_t value);

//[[script::bindings::operator(+)]]
    static sp<Integer> add(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Integer> sub(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Integer> mul(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Integer>& self, const sp<Integer>& rvalue);

//[[script::bindings::classmethod]]
    static void set(const sp<Integer>& self, const sp<Integer>& value);
//[[script::bindings::classmethod]]
    static void set(const sp<Integer>& self, int32_t value);
};

}

#endif
