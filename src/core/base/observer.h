#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Observer {
public:
    Observer(sp<Runnable> callback, bool oneshot = true);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Observer);

//  [[script::bindings::auto]]
    void notify();

//  [[script::bindings::property]]
    const sp<Runnable>& callback();
//  [[script::bindings::property]]
    void setCallback(sp<Runnable> callback);

private:
    sp<Runnable> _callback;
    bool _oneshot;

    friend class Notifier;
};

}
