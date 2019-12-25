#ifndef ARK_CORE_IMPL_NUMERIC_VIBRATE_H_
#define ARK_CORE_IMPL_NUMERIC_VIBRATE_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Vibrate : public Numeric {
public:
    Vibrate(const sp<Numeric>& t, float a, float c, float o);

    virtual float val() override;
    virtual bool update(uint64_t timestamp) override;

private:
    sp<Numeric> _t;
    float _a;
    float _c;
    float _o;
};

}

#endif
