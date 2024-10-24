#include "python/impl/adapter/event_listener_python.h"

#include "app/base/event.h"

#include "python/extension/py_cast.h"

#include "python/api.h"

namespace ark::plugin::python {

EventListenerPython::EventListenerPython(PyInstance callable)
    : _callable(std::move(callable)), _args(PyInstance::steal(PyTuple_New(1)))
{
}

bool EventListenerPython::onEvent(const Event& event)
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
        PythonExtension::instance().logErr();

    return consumed;
}

void EventListenerPython::traverse(const Debris::Visitor& visitor)
{
    visitor(_callable.ref());
}

}
