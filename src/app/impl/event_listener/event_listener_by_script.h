#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "app/inf/event_listener.h"

namespace ark {

class EventListenerByScript : public EventListener {
public:
    EventListenerByScript(const sp<Script>& script, const String& eventName);

    virtual bool onEvent(const Event& event) override;

private:
    sp<Script> _script;
    String _event_name;
};

}
