#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::class("EventListener")]]
class ARK_API EventListenerType {
public:
//  [[script::bindings::constructor]]
    static sp<EventListener> create(sp<EventListener> delegate = nullptr);

//  [[script::bindings::classmethod]]
    static bool onEvent(const sp<EventListener>& self, const Event& event);

//  [[script::bindings::classmethod]]
    static void reset(const sp<EventListener>& self, sp<EventListener> eventListener);

//  [[script::bindings::classmethod]]
    static void addEventListener(const sp<EventListener>& self, sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);
//  [[script::bindings::classmethod]]
    static void pushEventListener(const sp<EventListener>& self, sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);

private:
    static sp<EventListenerWrapper> ensureWrapper(const sp<EventListener>& self);
    static sp<EventListenerList> ensureEventListenerList(const sp<EventListener>& self);

};

}
