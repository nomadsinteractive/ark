#pragma once

#include "core/inf/debris.h"

#include "app/inf/event_listener.h"

#include "python/extension/py_instance.h"

namespace ark::plugin::python {

class EventListenerPython : public EventListener, public Debris, Implements<EventListenerPython, EventListener, Debris> {
public:
    EventListenerPython(PyInstance callable);

    bool onEvent(const Event& event) override;

    void traverse(const Visitor& visitor) override;

private:
    PyInstance _callable;
    PyInstance _args;
};

}
