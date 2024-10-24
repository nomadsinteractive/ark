#include "python/extension/py_instance_ref.h"

namespace ark::plugin::python {

PyInstanceRef::PyInstanceRef(PyObject* instance, bool ownership)
    : _instance(instance), _ownership(ownership)
{
}

PyInstanceRef::~PyInstanceRef()
{
    if(_ownership)
        Py_XDECREF(_instance);
}

PyObject* PyInstanceRef::instance() const
{
    return _instance;
}

void PyInstanceRef::clear()
{
    DCHECK(_ownership, "Cannot clear an instance you do not own");
    Py_CLEAR(_instance);
}

bool PyInstanceRef::hasOwnership() const
{
    return _ownership;
}

}
