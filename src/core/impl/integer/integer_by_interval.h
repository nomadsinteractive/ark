#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_BY_INTERVAL_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_BY_INTERVAL_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class IntegerByInterval : public Integer {
public:
    IntegerByInterval(const sp<Integer>& delegate, const sp<Numeric>& interval);

    virtual int32_t val() override;

};

}

#endif
