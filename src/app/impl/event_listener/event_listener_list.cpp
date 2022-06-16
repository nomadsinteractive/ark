#include "app/impl/event_listener/event_listener_list.h"

#include "app/base/event.h"

namespace ark {

void EventListenerList::addEventListener(const sp<EventListener>& eventListener, int32_t priority)
{
    DASSERT(eventListener);
    _event_listeners[-priority].push_back(eventListener);
}

bool EventListenerList::onEvent(const Event& event)
{
    for(auto iter = _event_listeners.begin(); iter != _event_listeners.end(); )
    {
        DList<EventListener>& eventListener = iter->second;
        if(eventListener.items().size() == 0)
            iter = _event_listeners.erase(iter);
        else
        {
            for(const sp<EventListener>& i : eventListener.update(event.timestamp()))
                if(i->onEvent(event))
                    return true;
            ++iter;
        }
    }
    return false;
}

}
