#include "app/util/event_listener_type.h"

#include "core/components/behavior.h"
#include "core/components/discarded.h"

#include "app/base/event.h"
#include "app/impl/event_listener/event_listener_list.h"
#include "app/impl/event_listener/event_listener_wrapper.h"

namespace ark {

namespace {

class EventListenerImpl final : public EventListener {
public:
    EventListenerImpl(sp<Behavior::Method> onEvent)
        : _on_event(std::move(onEvent))
    {
    }

    bool onEvent(const Event& event) override
    {
        Box arg1(sp<Event>::make(event));
        return static_cast<bool>(_on_event->call({std::move(arg1)}));
    }

private:
    sp<Behavior::Method> _on_event;
};

}

sp<EventListener> EventListenerType::create(sp<EventListener> delegate, StringView /*onEventName*/)
{
    return sp<EventListener>::make<EventListenerWrapper>(std::move(delegate));
}

sp<EventListener> EventListenerType::create(Behavior& delegate, StringView onEventName)
{
    return create(sp<EventListener>::make<EventListenerImpl>(delegate.getMethod(onEventName)), onEventName);
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
