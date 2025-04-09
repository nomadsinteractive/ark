#pragma once

#include "core/collection/list.h"

#include "app/inf/event_listener.h"

namespace ark {

class EventListenerList final : public EventListener {
public:
    virtual bool onEvent(const Event& event);

    void addEventListener(sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);
    void pushEventListener(sp<EventListener> eventListener, sp<Boolean> discarded = nullptr);

private:
    D_FList<sp<EventListener>> _event_listeners;
};

}
