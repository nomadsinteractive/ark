#ifndef ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_WRAPPER_H_
#define ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_WRAPPER_H_

#include "core/base/wrapper.h"

#include "app/inf/event_listener.h"

namespace ark {

class EventListenerWrapper : public EventListener, public Wrapper<EventListener> {
public:
    EventListenerWrapper(sp<EventListener> eventListener);

    virtual bool onEvent(const Event& event);

};

}

#endif
