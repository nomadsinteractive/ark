#ifndef ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_LIST_H_
#define ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_LIST_H_

#include <map>

#include "core/collection/list.h"

#include "app/inf/event_listener.h"

namespace ark {

class EventListenerList : public EventListener {
public:
    void addEventListener(sp<EventListener> eventListener, int32_t priority);

    virtual bool onEvent(const Event& event);

private:
    std::map<int32_t, DList<EventListener>> _event_listeners;
};

}

#endif
