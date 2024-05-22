#include "python/impl/adapter/python_callable_event_listener.h"

#include "app/base/event.h"

#include "python/extension/py_cast.h"

#include "python/api.h"

namespace ark::plugin::python {

PythonCallableEventListener::PythonCallableEventListener(PyInstance callable)
    : _callable(std::move(callable)), _args(PyInstance::steal(PyTuple_New(1)))
{
}

bool PythonCallableEventListener::onEvent(const Event& event)
{
    DCHECK_THREAD_FLAG();

    PyObject* pyEvent = PyCast::toPyObject(sp<Event>::make(event));
    PyTuple_SetItem(_args.pyObject(), 0, pyEvent);
    PyObject* ret = _callable.call(_args.pyObject());
    bool consumed = false;
    if(ret)
    {
        consumed = PyObject_IsTrue(ret) == 1;
        Py_DECREF(ret);
    }
    else
        PythonInterpreter::instance()->logErr();

    return consumed;
}

void PythonCallableEventListener::traverse(const Holder::Visitor& visitor)
{
    visitor(_callable.ref());
}

}
