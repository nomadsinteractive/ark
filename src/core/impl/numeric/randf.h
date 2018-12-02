#ifndef ARK_CORE_IMPL_NUMERIC_RANDF_H_
#define ARK_CORE_IMPL_NUMERIC_RANDF_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Randf : public Numeric {
public:
    Randf(const sp<Numeric>& b);
    Randf(const sp<Numeric>& a, const sp<Numeric>& b);

    virtual float val() override;

//  [[plugin::function("randf")]]
    static sp<Numeric> rand(const sp<Numeric>& b);
//  [[plugin::function("randf")]]
    static sp<Numeric> rand(const sp<Numeric>& a, const sp<Numeric>& b);

private:
    SafePtr<Numeric> _a;
    sp<Numeric> _b;
};

}

#endif
