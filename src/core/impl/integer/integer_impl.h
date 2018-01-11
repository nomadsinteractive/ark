#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_IMPL_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_IMPL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Integer")]]
class ARK_API IntegerImpl : public Integer {
public:
//  [[script::bindings::auto]]
    IntegerImpl(const sp<Integer>& delegate);
    IntegerImpl(int32_t value);

    virtual int32_t val() override;

private:
    int32_t _value;
    sp<Integer> _delegate;
};

}

#endif
