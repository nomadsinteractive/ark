#include "python/extension/py_bridge.h"

namespace ark {
namespace plugin {
namespace python {

PyObject* PyBridge::PyObject_GetItem(PyObject* obj, PyObject* key)
{
    return ::PyObject_GetItem(obj, key);
}

PyObject* PyBridge::PyModule_Create2(PyModuleDef* moduleDef, int32_t apiver)
{
    return ::PyModule_Create2(moduleDef, apiver);
}

int32_t PyBridge::PyModule_AddObject(PyObject* mod, const char* name, PyObject* value)
{
    return ::PyModule_AddObject(mod, name, value);
}

int32_t PyBridge::PyList_SetItem(PyObject* obj, Py_ssize_t idx, PyObject* item)
{
    return ::PyList_SetItem(obj, idx, item);
}

PyObject* PyBridge::PyList_New(Py_ssize_t size)
{
    return ::PyList_New(size);
}

PyObject* PyBridge::PyLong_FromLong(long value)
{
    return ::PyLong_FromLong(value);
}

PyObject* PyBridge::PyLong_FromUnsignedLong(unsigned long value)
{
    return ::PyLong_FromUnsignedLong(value);
}

PyObject* PyBridge::PyFloat_FromDouble(double value)
{
    return ::PyFloat_FromDouble(value);
}

Py_ssize_t PyBridge::PyObject_Size(PyObject* obj)
{
    return ::PyObject_Size(obj);
}

PyObject* PyBridge::PyObject_Type(PyObject* obj)
{
    return ::PyObject_Type(obj);
}

Py_ssize_t PyBridge::PyList_Size(PyObject* obj)
{
    return ::PyList_Size(obj);
}

PyObject* PyBridge::PyList_GetItem(PyObject* obj, Py_ssize_t idx)
{
    return ::PyList_GetItem(obj, idx);
}

PyObject* PyBridge::PyDict_Keys(PyObject* mp)
{
    return ::PyDict_Keys(mp);
}

PyObject* PyBridge::PyDict_GetItem(PyObject* mp, PyObject* key)
{
    return ::PyDict_GetItem(mp, key);
}

int32_t PyBridge::PyDict_SetItem(PyObject* mp, PyObject* key, PyObject* item)
{
    return ::PyDict_SetItem(mp, key, item);
}

int32_t PyBridge::PyArg_ParseTuple(PyObject* args, const char* format, ...)
{
    va_list val;
    va_start(val, format);
    int32_t r = PyArg_VaParse(args, format, val);
    va_end(val);
    return r;
}

int32_t PyBridge::PyArg_ParseTupleAndKeywords(PyObject* args, PyObject* kwargs, const char* format, char** argnames, ...)
{
    va_list val;
    va_start(val, argnames);
    int32_t r = PyArg_VaParseTupleAndKeywords(args, kwargs, format, argnames, val);
    va_end(val);
    return r;
}

void PyBridge::setRuntimeErrString(const char* string)
{
    PyErr_SetString(PyExc_RuntimeError, string);
}

void PyBridge::setTypeErrString(const char* string)
{
    PyErr_SetString(PyExc_TypeError, string);
}

PyObject* PyBridge::incRefNone()
{
    Py_RETURN_NONE;
}

void PyBridge::incRef(PyObject* obj)
{
    Py_INCREF(obj);
}

void PyBridge::decRef(PyObject* obj)
{
    Py_DECREF(obj);
}

bool PyBridge::isPyNone(PyObject* obj)
{
    return obj == Py_None;
}

bool PyBridge::isPyList(PyObject* obj)
{
    return PyList_Check(obj);
}

bool PyBridge::isPyBool(PyObject* obj)
{
    return PyBool_Check(obj);
}

bool PyBridge::isPyTuple(PyObject* obj)
{
    return PyTuple_Check(obj);
}

bool PyBridge::isPyDictExact(PyObject* obj)
{
    return PyDict_CheckExact(obj);
}

bool PyBridge::isPyListExact(PyObject* obj)
{
    return PyList_CheckExact(obj);
}

bool PyBridge::isPyLongExact(PyObject* obj)
{
    return PyLong_CheckExact(obj);
}

bool PyBridge::isPyFloatExact(PyObject* obj)
{
    return PyFloat_CheckExact(obj);
}

bool PyBridge::isPyTupleExact(PyObject* obj)
{
    return PyTuple_CheckExact(obj);
}

bool PyBridge::isPyUnicodeExact(PyObject* obj)
{
    return PyUnicode_CheckExact(obj);
}

}
}
}