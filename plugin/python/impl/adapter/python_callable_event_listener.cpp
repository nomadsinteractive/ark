#include "python/impl/adapter/python_callable_event_listener.h"

#include "app/base/event.h"

#include "python/extension/python_interpreter.h"

#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

PythonCallableEventListener::PythonCallableEventListener(const sp<PyInstance>& callable)
    : _callable(callable), _args(PyInstance::steal(PyTuple_New(1)))
{
}

bool PythonCallableEventListener::onEvent(const Event& event)
{
    DCHECK_THREAD_FLAG();

    PyObject* pyEvent = PythonInterpreter::instance()->fromSharedPtr<Event>(sp<Event>::make(event));
    PyTuple_SetItem(_args, 0, pyEvent);
    PyObject* ret = _callable->call(_args);
    bool consumed = false;
    if(ret)
    {
        consumed = ret == Py_None ? false : PyObject_IsTrue(ret) == 0;
        Py_DECREF(ret);
    }
    else
        PythonInterpreter::instance()->logErr();

    return consumed;
}

}
}
}
