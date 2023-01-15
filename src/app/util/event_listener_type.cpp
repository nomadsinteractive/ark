#include "app/util/event_listener_type.h"

#include "core/epi/disposed.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"

#include "app/impl/event_listener/event_listener_list.h"
#include "app/impl/event_listener/event_listener_wrapper.h"

namespace ark {

sp<EventListener> EventListenerType::create(sp<EventListener> eventListener)
{
    return sp<EventListenerWrapper>::make(std::move(eventListener));
}

bool EventListenerType::onEvent(const sp<EventListener>& self, const Event& event)
{
    return self->onEvent(event);
}

void EventListenerType::reset(const sp<EventListener>& self, sp<EventListener> eventListener)
{
    ensureWrapper(self)->reset(std::move(eventListener));
}

void EventListenerType::addEventListener(const sp<EventListener>& self, sp<EventListener> eventListener, sp<Boolean> disposed)
{
    ensureEventListenerList(self)->addEventListener(std::move(eventListener), std::move(disposed));
}

void EventListenerType::pushEventListener(const sp<EventListener>& self, sp<EventListener> eventListener, sp<Boolean> disposed)
{
    ensureEventListenerList(self)->pushEventListener(std::move(eventListener), std::move(disposed));
}

sp<EventListenerWrapper> EventListenerType::ensureWrapper(const sp<EventListener>& self)
{
    sp<EventListenerWrapper> wrapper = self.as<EventListenerWrapper>();
    CHECK(wrapper, "This EventListener object is not a EventListenerWrapper instance");
    return wrapper;
}

sp<EventListenerList> EventListenerType::ensureEventListenerList(const sp<EventListener>& self)
{
    sp<EventListenerWrapper> wrapper = ensureWrapper(self);
    sp<EventListener> wrapped = wrapper->wrapped();
    sp<EventListenerList> ell = wrapped ? wrapped.as<EventListenerList>() : nullptr;
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
