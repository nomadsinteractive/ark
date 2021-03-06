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
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::function("randf")]]
    static sp<Numeric> randf(const sp<Numeric>& b);
//  [[plugin::function("randf")]]
    static sp<Numeric> randf(const sp<Numeric>& a, const sp<Numeric>& b);
//  [[plugin::function("randfv")]]
    static sp<Numeric> randfv(const sp<Numeric>& b);
//  [[plugin::function("randfv")]]
    static sp<Numeric> randfv(const sp<Numeric>& a, const sp<Numeric>& b);

private:
    SafePtr<Numeric> _a;
    sp<Numeric> _b;
};

}

#endif
