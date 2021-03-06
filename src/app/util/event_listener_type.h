#ifndef ARK_APP_UTIL_EVENT_LISTENER_TYPE_H_
#define ARK_APP_UTIL_EVENT_LISTENER_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("EventListener")]]
class ARK_API EventListenerType {
public:
//[[script::bindings::constructor]]
    static sp<EventListener> create(const sp<EventListener>& eventListener);

//  [[script::bindings::classmethod]]
    static sp<EventListener> makeDisposable(const sp<EventListener>& self, const sp<Boolean>& disposed = nullptr);
//[[script::bindings::classmethod]]
    static sp<EventListener> makeAutoRelease(const sp<EventListener>& self, int32_t refCount = 1);


//  [[plugin::builder]]
    class BUILDER : public Builder<EventListener> {
    public:
        BUILDER() = default;

        virtual sp<EventListener> build(const Scope& args) override;

    };
};

}

#endif
