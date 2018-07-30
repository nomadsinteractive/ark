#ifndef ARK_CORE_IMPL_NUMERIC_MIN_H_
#define ARK_CORE_IMPL_NUMERIC_MIN_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Min : public Numeric {
public:
    Min(const sp<Numeric>& a1, const sp<Numeric>& a2);

    virtual float val() override;

//  [[plugin::function("min")]]
    static sp<Numeric> min(const sp<Numeric>& a, const sp<Numeric>& b);

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

}

#endif
