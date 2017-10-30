#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_VARIABLE_TYPE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_ARK_VARIABLE_TYPE_H_

#include "python/extension/py_ark_type.h"

namespace ark {
namespace plugin {
namespace python {

class PyArkNumericType : public PyArkType {
public:
    PyArkNumericType(const String& name, const String& doc, long flags);

    static PyObject* val(Instance* self, PyObject* args);
    static PyObject* set(Instance* self, PyObject *args);

    static PyObject* __add__(Instance* self, PyObject *args);
    static PyObject* __sub__(Instance* self, PyObject *args);
    static PyObject* __mul__(Instance* self, PyObject *args);
    static PyObject* __neg__(Instance* self);

    static PyObject* __div__(Instance* self, PyObject *args);
    static PyObject* __truediv__(Instance* self, PyObject *args);

    static PyObject* __float__(Instance* self);
    static PyObject* __iadd__(Instance* self, PyObject *args);
    static PyObject* __isub__(Instance* self, PyObject *args);
    static PyObject* __imul__(Instance* self, PyObject *args);
    static PyObject* __idiv__(Instance* self, PyObject *args);

private:
    static int __init__(Instance* self, PyObject* args, PyObject* kwargs);

};

}
}
}

#endif
