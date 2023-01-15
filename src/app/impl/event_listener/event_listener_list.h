#ifndef ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_LIST_H_
#define ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_LIST_H_

#include "core/collection/list.h"

#include "app/inf/event_listener.h"

namespace ark {

class EventListenerList : public EventListener {
public:
    virtual bool onEvent(const Event& event);

    void addEventListener(sp<EventListener> eventListener, sp<Boolean> disposed = nullptr);
    void pushEventListener(sp<EventListener> eventListener, sp<Boolean> disposed = nullptr);

private:
    DList<EventListener> _event_listeners;
};

}

#endif
