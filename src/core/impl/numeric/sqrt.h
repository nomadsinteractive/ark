#ifndef ARK_CORE_IMPL_NUMERIC_SQRT_H_
#define ARK_CORE_IMPL_NUMERIC_SQRT_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Sqrt : public Numeric {
public:
    Sqrt(const sp<Numeric>& x);

    virtual float val() override;

//  [[plugin::function("sqrt")]]
    static sp<Numeric> call(const sp<Numeric>& x);

private:
    sp<Numeric> _x;
};

}

#endif
