#pragma once

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Randf final : public Numeric {
public:
    Randf(sp<Numeric> b);
    Randf(sp<Numeric> a, sp<Numeric> b);

    float val() override;
    bool update(uint64_t timestamp) override;

    static sp<Numeric> randf(const sp<Numeric>& b);
    static sp<Numeric> randf(const sp<Numeric>& a, const sp<Numeric>& b);
    static sp<Numeric> randfv(const sp<Numeric>& b);
    static sp<Numeric> randfv(const sp<Numeric>& a, const sp<Numeric>& b);

private:
    SafeVar<Numeric> _a;
    sp<Numeric> _b;
};

}
