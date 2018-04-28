#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

PyInstance::PyInstance(const PyInstance& other)
    : _object(other._object), _is_borrowed_reference(other._is_borrowed_reference)
{
    if(!_is_borrowed_reference)
        Py_XINCREF(_object);
}

PyInstance::PyInstance(const PyInstance::Instance& instance)
    : _object(instance._object), _is_borrowed_reference(instance._is_borrowed_reference)
{
}

PyInstance::PyInstance(PyObject* object, bool isBorrowedReference)
    : _object(object), _is_borrowed_reference(isBorrowedReference)
{
}

PyInstance::~PyInstance()
{
    if(!_is_borrowed_reference)
        Py_XDECREF(_object);
}

PyInstance::Instance PyInstance::borrow(PyObject* object)
{
    return Instance(object, true);
}

PyInstance::Instance PyInstance::steal(PyObject* object)
{
    return Instance(object, false);
}

PyInstance::Instance PyInstance::adopt(PyObject* object)
{
    Py_XINCREF(object);
    return Instance(object, false);
}

PyObject* PyInstance::instance()
{
    return _object;
}

PyInstance::operator bool()
{
    return _object && Py_None != _object;
}

PyObject* PyInstance::type()
{
    return reinterpret_cast<PyObject*>(_object->ob_type);
}

const char* PyInstance::name()
{
    return Py_TYPE(_object)->tp_name;
}

PyInstance::operator PyObject* ()
{
    return _object;
}

bool PyInstance::hasAttr(const char* name) const
{
    return PyObject_HasAttrString(_object, name) != 0;
}

PyObject* PyInstance::getAttr(const char* name) const
{
    return PyObject_GetAttrString(_object, name);
}

PyObject* PyInstance::getAttr(PyObject* name) const
{
    return PyObject_GetAttr(_object, name);
}

void PyInstance::setAttr(const char* name, PyObject* attr)
{
    PyObject_SetAttrString(_object, name, attr);
}

int PyInstance::setAttr(PyObject* name, PyObject* attr)
{
    return PyObject_SetAttr(_object, name, attr);
}

PyObject* PyInstance::call(PyObject* args)
{
    return PyObject_Call(_object, args, nullptr);
}

bool PyInstance::isCallable()
{
    return PyCallable_Check(_object) != 0;
}

long PyInstance::hash()
{
    return PyObject_Hash(_object);
}

PyObject* PyInstance::release()
{
    PyObject* obj = _object;
    _object = nullptr;
    return obj;
}

PyInstance::Instance::Instance(PyObject* object, bool isBorrowedReference)
    : _object(object), _is_borrowed_reference(isBorrowedReference)
{
}

PyInstance::Instance::Instance(const PyInstance::Instance& other)
    : _object(other._object), _is_borrowed_reference(other._is_borrowed_reference)
{
}

}
}
}
