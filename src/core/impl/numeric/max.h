#ifndef ARK_CORE_IMPL_NUMERIC_MAX_H_
#define ARK_CORE_IMPL_NUMERIC_MAX_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Max : public Numeric {
public:
    Max(const sp<Numeric>& a1, const sp<Numeric>& a2);

    virtual float val() override;

//  [[plugin::function("max")]]
    static sp<Numeric> max(const sp<Numeric>& a, const sp<Numeric>& b);

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

}

#endif
