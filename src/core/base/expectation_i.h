#ifndef ARK_CORE_BASE_EXPECTATION_I_H_
#define ARK_CORE_BASE_EXPECTATION_I_H_

#include "core/base/expectation.h"

namespace ark {

typedef Expectation<int32_t> ExpectationI;

/*

//[[script::bindings::holder]]
//[[script::bindings::extends(Integer)]]
class ARK_API ExpectationI : public Integer, public Wrapper<Integer>, public Holder {
 public:
    ExpectationI(sp<Integer> delegate, Notifier notifier);

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
