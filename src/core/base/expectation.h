#ifndef ARK_CORE_BASE_EXPECTATION_H_
#define ARK_CORE_BASE_EXPECTATION_H_

#include "core/base/api.h"
#include "core/epi/notifier.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Expectation : public Numeric {
public:
    Expectation(const sp<Numeric>& delegate, const sp<Numeric>& expectation, Notifier notifier);

    virtual float val() override;

    const sp<Numeric>& expectation() const;

    const Notifier& notifier() const;

//[[script::bindings::auto]]
    void update();

//[[script::bindings::auto]]
    sp<Observer> createObserver(const sp<Runnable>& callback, bool oneshot = false);

//[[script::bindings::auto]]
    const sp<Observer>& addObserver(const sp<Runnable>& callback, bool oneshot = false);

private:
    sp<Numeric> _delegate;
    sp<Numeric> _expectation;

    Notifier _notifier;

    std::vector<sp<Observer>> _observers;
};

}

#endif
