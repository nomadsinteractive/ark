#include "app/impl/event_listener/event_listener_by_script.h"

#include "core/inf/interpreter.h"

#include "app/base/event.h"

namespace ark {

EventListenerByScript::EventListenerByScript(const sp<Interpreter>& script, const String& eventName)
    : _script(script), _func(_script->attr(nullptr, eventName))
{
}

bool EventListenerByScript::onEvent(const Event& event)
{
    Interpreter::Arguments args;
    args.emplace_back(sp<Event>::make(event));
    return static_cast<bool>(_script->call(_func, args));
}

}
