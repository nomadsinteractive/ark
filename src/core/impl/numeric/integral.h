#ifndef ARK_CORE_IMPL_NUMERIC_INTEGRAL_H_
#define ARK_CORE_IMPL_NUMERIC_INTEGRAL_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Integral : public Numeric {
public:
    Integral(const sp<Numeric>& v, const sp<Numeric>& t);

    virtual float val() override;

private:
    sp<Numeric> _v;
    sp<Numeric> _t;

    float _s;
    float _last_v;
    float _last_t;
};

}

#endif
