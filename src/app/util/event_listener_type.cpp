#include "app/util/event_listener_type.h"

#include "app/impl/event_listener/event_listener_wrapper.h"

namespace ark {

sp<EventListener> EventListenerType::wrap(sp<EventListener> delegate)
{
    return sp<EventListener>::make<EventListenerWrapper>(std::move(delegate));
}

bool EventListenerType::onEvent(const sp<EventListener>& self, const Event& event)
{
    return self->onEvent(event);
}

void EventListenerType::reset(const sp<EventListener>& self, sp<EventListener> eventListener)
{
    self.ensureInstance<EventListenerWrapper>("This EventListener object is not a EventListenerWrapper instance")->reset(std::move(eventListener));
}

}
