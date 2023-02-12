#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PY_BRIDGE_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PY_BRIDGE_H_

#include <Python.h>

#include "core/forwarding.h"

#include "python/forwarding.h"
#include "python/api.h"

namespace ark {
namespace plugin {
namespace python {

class ARK_PLUGIN_PYTHON_API PyBridge {
public:
    static PyObject* PyObject_GetItem(PyObject* obj, PyObject* key);
    static PyObject* PyModule_Create2(struct PyModuleDef* moduleDef, int32_t apiver);
    static int32_t PyModule_AddObject(PyObject* mod, const char* name, PyObject* value);
    static int32_t PyList_SetItem(PyObject* obj, Py_ssize_t idx, PyObject* item);
    static PyObject* PyLong_FromLong(long);
    static PyObject* PyLong_FromLongLong(long long value);
    static PyObject* PyLong_FromUnsignedLong(unsigned long);
    static PyObject* PyLong_FromUnsignedLongLong(unsigned long long);
    static PyObject* PyFloat_FromDouble(double);

    static Py_ssize_t PyObject_Size(PyObject* obj);
    static PyObject* PyObject_Type(PyObject* obj);
    static PyObject* PyObject_GetIter(PyObject *obj);

    static PyObject* PyList_New(Py_ssize_t size);
    static PyObject* PyList_GetItem(PyObject* obj, Py_ssize_t idx);
    static Py_ssize_t PyList_Size(PyObject* obj);

    static PyObject* PyIter_Next(PyObject* iter);

    static PyObject* PyDict_Keys(PyObject* mp);
    static PyObject* PyDict_GetItem(PyObject* mp, PyObject* key);
    static int PyDict_SetItem(PyObject* mp, PyObject* key, PyObject* item);

    static int32_t PyArg_ParseTuple(PyObject* args, const char* format, ...);
    static int32_t PyArg_ParseTupleAndKeywords(PyObject*args, PyObject* kwargs, const char* format, char** argnames, ...);

    static void setRuntimeErrString(const char* string);
    static void setTypeErrString(const char* string);

    static PyObject* incRefNone();

    static void incRef(PyObject* obj);
    static void decRef(PyObject* obj);

    static bool isPyNone(PyObject* obj);
    static bool isPyList(PyObject* obj);
    static bool isPyBool(PyObject* obj);
    static bool isPyTuple(PyObject* obj);

    static bool isPyDictExact(PyObject* obj);
    static bool isPyListExact(PyObject* obj);
    static bool isPyLongExact(PyObject* obj);
    static bool isPyFloatExact(PyObject* obj);
    static bool isPyTupleExact(PyObject* obj);
    static bool isPyUnicodeExact(PyObject* obj);
};

}
}
}

#endif
