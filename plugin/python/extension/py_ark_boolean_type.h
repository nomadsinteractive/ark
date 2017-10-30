#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_BOOLEAN_TYPE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_BOOLEAN_TYPE_H_

#include "python/extension/py_ark_type.h"

namespace ark {
namespace plugin {
namespace python {

class PyArkBooleanType : public PyArkType {
public:
    PyArkBooleanType(const String& name, const String& doc, long flags);

    static PyObject* val(Instance* self, PyObject* args);
    static PyObject* set(Instance* self, PyObject *args);

    static int __bool__(Instance* self);

private:
    static int __init__(Instance* self, PyObject* args, PyObject* kwargs);
};

}
}
}

#endif
