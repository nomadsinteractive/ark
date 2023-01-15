#include "app/impl/event_listener/event_listener_wrapper.h"

namespace ark {

EventListenerWrapper::EventListenerWrapper(sp<EventListener> eventListener)
    : Wrapper(std::move(eventListener))
{
}

bool EventListenerWrapper::onEvent(const Event& event)
{
    return _wrapped ? _wrapped->onEvent(event) : false;
}

}
