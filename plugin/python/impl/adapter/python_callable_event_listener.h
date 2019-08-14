#ifndef ARK_PLUGIN_PYTHON_IMPL_EVENT_LISTENER_PYTHON_CALLABLE_EVENT_LISTENER_H_
#define ARK_PLUGIN_PYTHON_IMPL_EVENT_LISTENER_PYTHON_CALLABLE_EVENT_LISTENER_H_

#include "core/types/shared_ptr.h"

#include "app/inf/event_listener.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class PythonCallableEventListener : public EventListener, Implements<PythonCallableEventListener, EventListener> {
public:
    PythonCallableEventListener(PyInstance callable);

    virtual bool onEvent(const Event& event) override;

private:
    PyInstance _callable;
    PyInstance _args;

};

}
}
}

#endif
