#ifndef ARK_CORE_BASE_OBSERVER_H_
#define ARK_CORE_BASE_OBSERVER_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Observer : public Boolean {
public:
//  [[script::bindings::auto]]
    Observer(const sp<Runnable>& callback, bool oneshot = true);
    Observer(bool dirty = true, const sp<Runnable>& callback = nullptr, bool oneshot = false);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Observer);

    virtual bool val() override;

//  [[script::bindings::auto]]
    bool dirty();

//  [[script::bindings::auto]]
    void update();

//  [[script::bindings::auto]]
    void setCallback(const sp<Runnable>& callback);

private:
    sp<Runnable> _callback;
    bool _oneshot;
    bool _dirty;

    friend class Notifier;
};

}

#endif
