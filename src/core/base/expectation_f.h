#ifndef ARK_CORE_BASE_EXPECTATION_F_H_
#define ARK_CORE_BASE_EXPECTATION_F_H_

#include "core/base/expectation.h"

namespace ark {

typedef Expectation<float> ExpectationF;

/*

//[[script::bindings::holder]]
//[[script::bindings::extends(Numeric)]]
class ARK_API ExpectationF : public Numeric, public Wrapper<Numeric>, public Holder {
 public:
    ExpectationF(sp<Numeric> delegate, Notifier notifier);

    virtual float val() override;
    virtual bool update(uint64_t timestamp) override;
    virtual void traverse(const Visitor& visitor) override;

//[[script::bindings::auto]]
    sp<Observer> createObserver(const sp<Runnable>& callback, bool oneshot = true);
//[[script::bindings::auto]]
    const sp<Observer>& addObserver(const sp<Runnable>& callback, bool oneshot = true);
//[[script::bindings::auto]]
    void clear();
};

*/

}

#endif
