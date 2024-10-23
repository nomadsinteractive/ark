#include "python/extension/py_instance_ref.h"

namespace ark::plugin::python {

PyInstanceRef::PyInstanceRef(PyObject* instance, bool deref)
    : _instance(instance), _deref(deref)
{
}

PyInstanceRef::~PyInstanceRef()
{
    if(_deref)
        Py_XDECREF(_instance);
}

PyObject* PyInstanceRef::instance() const
{
    return _instance;
}

void PyInstanceRef::clear()
{
    DCHECK(_deref, "Cannot clear an instance you do not own");
    Py_CLEAR(_instance);
}

}
