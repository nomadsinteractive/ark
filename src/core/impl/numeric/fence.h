#ifndef ARK_CORE_IMPL_NUMERIC_FENCE_H_
#define ARK_CORE_IMPL_NUMERIC_FENCE_H_

#include "core/forwarding.h"
#include "core/epi/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Fence : public Numeric {
public:
    Fence(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier);

    virtual float val() override;

private:
    sp<Numeric> _delegate;
    sp<Numeric> _expectation;

    Notifier _notifer;

    bool _is_greater;
};

}

#endif
