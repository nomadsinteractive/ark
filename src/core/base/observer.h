#ifndef ARK_CORE_BASE_OBSERVER_H_
#define ARK_CORE_BASE_OBSERVER_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Observer : public Boolean {
public:
    Observer(const sp<Runnable>& callback, bool oneshot);
    Observer(bool dirty = true, const sp<Runnable>& callback = nullptr, bool oneshot = false);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Observer);

    virtual bool val() override;

    bool dirty();

    void update();

private:
    sp<Runnable> _callback;
    bool _oneshot;
    bool _dirty;

    friend class Notifier;
};

}

#endif
