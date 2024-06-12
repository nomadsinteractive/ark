#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "app/inf/event_listener.h"

namespace ark {

class EventListenerByScript : public EventListener {
public:
    EventListenerByScript(const sp<Interpreter>& script, const String& eventName);

    virtual bool onEvent(const Event& event) override;

private:
    sp<Interpreter> _script;
    Box _func;
};

}
