#include "app/impl/event_listener/event_listener_list.h"

#include "core/base/enum.h"
#include "core/components/discarded.h"

#include "app/base/event.h"
#include "core/collection/traits.h"

namespace ark {

void EventListenerList::addEventListener(sp<EventListener> eventListener, const Traits& traits)
{
    ASSERT(eventListener);
    const enums::InsertPosition ip = traits.getEnum<enums::InsertPosition>(enums::InsertPosition::INSERT_POSITION_BACK);
    const sp<Discarded> discarded = traits.get<Discarded>();
    if(ip == enums::INSERT_POSITION_BACK)
        _event_listeners.emplace_back(std::move(eventListener), discarded);
    else
        _event_listeners.emplace_front(std::move(eventListener), discarded);
}

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
