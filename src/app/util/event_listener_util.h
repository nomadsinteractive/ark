#ifndef ARK_APP_UTIL_EVENT_LISTENER_UTIL_H_
#define ARK_APP_UTIL_EVENT_LISTENER_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("EventListener")]]
class ARK_API EventListenerUtil {
public:
//[[script::bindings::constructor]]
    static sp<EventListener> create(const sp<EventListener>& eventListener);

//  [[script::bindings::classmethod]]
    static sp<EventListener> makeDisposable(const sp<EventListener>& self, const sp<Boolean>& disposed = nullptr);
//[[script::bindings::classmethod]]
    static sp<EventListener> makeAutoRelease(const sp<EventListener>& self, int32_t refCount = 1);

};

}

#endif
