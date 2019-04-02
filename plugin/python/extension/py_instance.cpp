#include "python/extension/py_instance.h"

#include "core/types/shared_ptr.h"

#include "extension/python_interpreter.h"
#include "extension/reference_manager.h"

namespace ark {
namespace plugin {
namespace python {


PyInstance::PyInstance(PyObject* object, bool isBorrowedReference)
    : _object(object), _is_borrowed_reference(isBorrowedReference)
{
}

PyInstance::PyInstance(PyInstance&& other)
    : _object(other._object), _is_borrowed_reference(other._is_borrowed_reference)
{
    other._object = nullptr;
}

PyInstance::~PyInstance()
{
    if(!_is_borrowed_reference)
        Py_XDECREF(_object);
}

PyInstance PyInstance::borrow(PyObject* object)
{
    return PyInstance(object, true);
}

PyInstance PyInstance::steal(PyObject* object)
{
    return PyInstance(object, false);
}

PyInstance PyInstance::own(PyObject* object)
{
    Py_XINCREF(object);
    return PyInstance(object, false);
}

sp<PyInstance> PyInstance::track(PyObject* object)
{
    Py_XINCREF(object);
    const sp<PyInstance> ref = sp<PyInstance>::adopt(new PyInstance(object, false));
    PythonInterpreter::instance()->referenceManager()->track(ref);
    return ref;
}

PyObject* PyInstance::object()
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

sp<PyInstance> PyInstance::getAttr(const char* name) const
{
    const sp<PyInstance> attr = sp<PyInstance>::make(PyInstance::steal(PyObject_GetAttrString(_object, name)));
    PythonInterpreter::instance()->referenceManager()->track(attr);
    return attr;
}

PyObject* PyInstance::call(PyObject* args)
{
    return PyObject_Call(_object, args, nullptr);
}

bool PyInstance::isCallable()
{
    return PyCallable_Check(_object) != 0;
}

void PyInstance::clear()
{
    if(_object)
    {
        DASSERT(_object->ob_refcnt > 0);
        Py_DECREF(_object);
        _object = nullptr;
    }
}

}
}
}
