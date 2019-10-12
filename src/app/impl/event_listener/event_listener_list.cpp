#include "app/impl/event_listener/event_listener_list.h"

namespace ark {

void EventListenerList::addEventListener(const sp<EventListener>& eventListener)
{
    DASSERT(eventListener);
    _event_listeners.push_back(eventListener);
}

bool EventListenerList::onEvent(const Event& event)
{
    for(const sp<EventListener>& i : _event_listeners)
        if(i->onEvent(event))
            return true;
    return false;
}

EventListenerList::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& /*manifest*/)
{
}

sp<EventListener> EventListenerList::BUILDER::build(const Scope& /*args*/)
{
    return sp<EventListenerList>::make();
}

}
