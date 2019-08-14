#include "python/extension/py_instance_ref.h"

namespace ark {
namespace plugin {
namespace python {

PyInstanceRef::PyInstanceRef(PyObject* instance)
    : _instance(instance)
{
}

PyObject* PyInstanceRef::instance()
{
    return _instance;
}

void PyInstanceRef::clear()
{
    Py_CLEAR(_instance);
}

}
}
}
