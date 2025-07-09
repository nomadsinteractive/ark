#include "app/util/event_listener_type.h"

#include "core/components/discarded.h"

#include "app/impl/event_listener/event_listener_list.h"
#include "app/impl/event_listener/event_listener_wrapper.h"

namespace ark {

sp<EventListener> EventListenerType::create(sp<EventListener> delegate)
{
    return sp<EventListener>::make<EventListenerWrapper>(std::move(delegate));
}

bool EventListenerType::onEvent(const sp<EventListener>& self, const Event& event)
{
    return self->onEvent(event);
}

void EventListenerType::reset(const sp<EventListener>& self, sp<EventListener> eventListener)
{
    ensureWrapper(self)->reset(std::move(eventListener));
}

void EventListenerType::addEventListener(const sp<EventListener>& self, sp<EventListener> eventListener, sp<Boolean> discarded)
{
    ensureEventListenerList(self)->addEventListener(std::move(eventListener), std::move(discarded));
}

void EventListenerType::pushEventListener(const sp<EventListener>& self, sp<EventListener> eventListener, sp<Boolean> discarded)
{
    ensureEventListenerList(self)->pushEventListener(std::move(eventListener), std::move(discarded));
}

sp<EventListenerWrapper> EventListenerType::ensureWrapper(const sp<EventListener>& self)
{
    return self.ensureInstance<EventListenerWrapper>("This EventListener object is not a EventListenerWrapper instance");
}

sp<EventListenerList> EventListenerType::ensureEventListenerList(const sp<EventListener>& self)
{
    const sp<EventListenerWrapper> wrapper = ensureWrapper(self);
    sp<EventListener> wrapped = wrapper->wrapped();
    sp<EventListenerList> ell = wrapped ? wrapped.asInstance<EventListenerList>() : nullptr;
    if(!ell)
    {
        ell = sp<EventListenerList>::make();
        if(wrapped)
            ell->addEventListener(std::move(wrapped));
        wrapper->reset(ell);
    }
    return ell;
}

}
