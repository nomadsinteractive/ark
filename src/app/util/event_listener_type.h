#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("EventListener")]]
class ARK_API EventListenerType {
public:
//  [[script::bindings::classmethod]]
    static sp<EventListener> wrap(sp<EventListener> delegate);

//  [[script::bindings::classmethod]]
    static bool onEvent(const sp<EventListener>& self, const Event& event);

//  [[script::bindings::classmethod]]
    static void reset(const sp<EventListener>& self, sp<EventListener> eventListener);

};

}
