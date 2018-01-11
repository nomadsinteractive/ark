#include "app/impl/event_listener/event_listener_by_script.h"

#include "core/inf/script.h"

#include "app/base/event.h"

namespace ark {

EventListenerByScript::EventListenerByScript(const sp<Script>& script, const String& eventName)
    : _script(script), _event_name(eventName)
{
}

bool EventListenerByScript::onEvent(const Event& event)
{
    const sp<Event> copy = sp<Event>::make(event);
    Script::Arguments args;
    args.push_back(copy.pack());
    return static_cast<bool>(_script->call(_event_name, args));
}

}
