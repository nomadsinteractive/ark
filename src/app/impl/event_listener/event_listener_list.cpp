#include "app/impl/event_listener/event_listener_list.h"

#include "app/base/event.h"

namespace ark {

void EventListenerList::addEventListener(sp<EventListener> eventListener, sp<Boolean> disposed)
{
    ASSERT(eventListener);
    _event_listeners.emplace_back(std::move(eventListener), std::move(disposed));
}

void EventListenerList::pushEventListener(sp<EventListener> eventListener, sp<Boolean> disposed)
{
    ASSERT(eventListener);
    _event_listeners.emplace_front(std::move(eventListener), std::move(disposed));
}

bool EventListenerList::onEvent(const Event& event)
{
    for(EventListener& i : _event_listeners.update(event.timestamp()))
        if(i.onEvent(event))
            return true;
    return false;
}

}
