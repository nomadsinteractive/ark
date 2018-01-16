#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_SUBTRACT_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_SUBTRACT_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class IntegerSubtract : public Integer {
public:
    IntegerSubtract(const sp<Integer>& lvalue, const sp<Integer>& rvalue);

    virtual int32_t val() override;

private:
    sp<Integer> _lvalue;
    sp<Integer> _rvalue;
};

}

#endif
