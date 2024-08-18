#include "app/impl/event_listener/event_listener_list.h"

#include "app/base/event.h"

namespace ark {

void EventListenerList::addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    ASSERT(eventListener);
    _event_listeners.emplace_back(std::move(eventListener), std::move(discarded));
}

void EventListenerList::pushEventListener(sp<EventListener> eventListener, sp<Boolean> discarded)
{
    ASSERT(eventListener);
    _event_listeners.emplace_front(std::move(eventListener), std::move(discarded));
}

bool EventListenerList::onEvent(const Event& event)
{
    for(EventListener& i : _event_listeners.update(event.timestamp()))
        if(i.onEvent(event))
            return true;
    return false;
}

}
