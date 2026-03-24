#include "python/extension/py_instance_ref.h"

#include "python_extension.h"

namespace ark::plugin::python {

PyInstanceRef::PyInstanceRef(PyObject* instance, const bool ownership)
    : _instance(instance), _ownership(ownership)
{
}

PyInstanceRef::~PyInstanceRef()
{
    if(_ownership)
    {
        const auto gil = PythonExtension::instance().ensureGIL();
        Py_XDECREF(_instance);
    }
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
