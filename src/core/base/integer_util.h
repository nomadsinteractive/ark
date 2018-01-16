#ifndef ARK_CORE_BASE_INTEGER_UTIL_H_
#define ARK_CORE_BASE_INTEGER_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/impl/integer/integer_wrapper.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Integer")]]
class ARK_API IntegerUtil {
public:

//[[script::bindings::constructor]]
    static sp<Integer> create(const sp<Integer>& value);
//[[script::bindings::constructor]]
    static sp<Integer> create(const int32_t value);

//[[script::bindings::operator(+)]]
    static sp<Integer> add(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Integer> sub(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Integer> mul(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Integer> negative(const sp<Integer>& self);
//[[script::bindings::operator(int)]]
    static int32_t toInt32(const sp<Integer>& self);

//[[script::bindings::classmethod]]
    static void set(const sp<Integer::Impl>& self, int32_t value);
//[[script::bindings::classmethod]]
    static void set(const sp<IntegerWrapper>& self, int32_t value);
//[[script::bindings::classmethod]]
    static void set(const sp<IntegerWrapper>& self, const sp<Integer>& delegate);

//[[script::bindings::classmethod]]
    static void fix(const sp<Integer>& self);
};

}

#endif
