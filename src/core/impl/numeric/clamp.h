#ifndef ARK_CORE_IMPL_NUMERIC_CLAMP_H_
#define ARK_CORE_IMPL_NUMERIC_CLAMP_H_

#include "core/forwarding.h"
#include "core/epi/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Clamp : public Numeric::Updatable {
public:
    Clamp(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, Notifier notifier);

    virtual bool doUpdate(uint64_t timestamp, float& value) override;

private:
    sp<Numeric> _delegate;
    sp<Numeric> _min;
    sp<Numeric> _max;

    Notifier _notifier;
};

}

#endif
