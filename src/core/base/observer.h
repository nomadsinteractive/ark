#ifndef ARK_CORE_BASE_OBSERVER_H_
#define ARK_CORE_BASE_OBSERVER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/holder.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Observer : public Holder {
public:
//  [[script::bindings::auto]]
    Observer(sp<Runnable> callback, bool oneshot = true);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Observer);

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::auto]]
    void notify();

//  [[script::bindings::auto]]
    void setCallback(sp<Runnable> callback);

private:
    sp<Runnable> _callback;
    bool _oneshot;

    friend class Notifier;
};

}

#endif
