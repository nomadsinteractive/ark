#ifndef ARK_CORE_IMPL_NUMERIC_APPROACH_H_
#define ARK_CORE_IMPL_NUMERIC_APPROACH_H_

#include "core/forwarding.h"
#include "core/base/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Approach : public Numeric {
public:
    Approach(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier);

    virtual float val() override;
    virtual bool update(uint64_t timestamp) override;

private:
    sp<Numeric> _delegate;
    sp<Numeric> _expectation;

    Notifier _notifer;

    float _delegate_value;
};

}

#endif
