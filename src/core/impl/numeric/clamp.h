#ifndef ARK_CORE_IMPL_NUMERIC_CLAMP_H_
#define ARK_CORE_IMPL_NUMERIC_CLAMP_H_

#include "core/forwarding.h"
#include "core/epi/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Clamp : public Numeric {
public:
    Clamp(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, Notifier notifier);

    virtual float val() override;

private:
    sp<Numeric> _delegate;
    sp<Numeric> _min;
    sp<Numeric> _max;

    Notifier _notifier;
};

}

#endif
