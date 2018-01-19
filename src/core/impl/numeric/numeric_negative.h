#ifndef ARK_CORE_IMPL_NUMERIC_NUMERIC_NEGATIVE_H_
#define ARK_CORE_IMPL_NUMERIC_NUMERIC_NEGATIVE_H_

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class NumericNegative : public Numeric {
public:
    NumericNegative(const sp<Numeric>& a1);

    virtual float val() override;

private:
    sp<Numeric> _a1;

};

}

#endif
