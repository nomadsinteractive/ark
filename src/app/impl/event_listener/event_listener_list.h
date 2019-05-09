#ifndef ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_LIST_H_
#define ARK_APP_IMPL_EVENT_LISTENER_EVENT_LISTENER_LIST_H_

#include "core/collection/list.h"

#include "app/inf/event_listener.h"

namespace ark {

class EventListenerList : public EventListener {
public:
    void addEventListener(const sp<EventListener>& eventListener);

    virtual bool onEvent(const Event& event);

//  [[plugin::builder]]
    class BUILDER : public Builder<EventListener> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<EventListener> build(const sp<Scope>& args) override;

    };

private:
    DisposableItemList<EventListener> _event_listeners;
};

}

#endif
