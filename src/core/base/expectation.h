#ifndef ARK_CORE_BASE_EXPECTATION_H_
#define ARK_CORE_BASE_EXPECTATION_H_

#include "core/base/api.h"
#include "core/base/delegate.h"
#include "core/epi/notifier.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::holder]]
//[[script::bindings::extends(Numeric)]]
class ARK_API Expectation : public Numeric, public Delegate<Numeric>, public Holder {
public:
    Expectation(const sp<Numeric>& delegate, Notifier notifier);

    virtual float val() override;
    virtual int32_t traverse(const Visitor& visitor) override;
    virtual int32_t clear() override;

//[[script::bindings::auto]]
    sp<Observer> createObserver(const sp<Runnable>& callback, bool oneshot = false);

//[[script::bindings::auto]]
    const sp<Observer>& addObserver(const sp<Runnable>& callback, bool oneshot = false);

private:
    Notifier _notifier;
    std::vector<sp<Observer>> _observers;

};

}

#endif
