#include "python/extension/py_instance.h"

#include "core/types/shared_ptr.h"
#include "core/util/log.h"

#include "extension/python_interpreter.h"
#include "extension/reference_manager.h"

namespace ark {
namespace plugin {
namespace python {

PyInstance::PyInstance()
{
}

PyInstance::PyInstance(const sp<PyInstanceRef>& ref)
    : _ref(ref)
{
}

PyInstance PyInstance::borrow(PyObject* object)
{
    return PyInstance(sp<Borrowed>::make(object));
}

PyInstance PyInstance::steal(PyObject* object)
{
    return PyInstance(sp<Stolen>::make(object));
}

PyInstance PyInstance::own(PyObject* object)
{
    return PyInstance(sp<Owned>::make(object));
}

PyInstance PyInstance::track(PyObject* object)
{
    const sp<PyInstanceRef> ref = sp<Owned>::make(object);
    PythonInterpreter::instance()->referenceManager()->track(ref);
    return ref;
}

const sp<PyInstanceRef>& PyInstance::ref() const
{
    return _ref;
}

PyInstance::operator bool()
{
    return _ref && !isNone();
}

PyObject* PyInstance::type()
{
    return reinterpret_cast<PyObject*>(_ref->instance()->ob_type);
}

const char* PyInstance::name()
{
    return Py_TYPE(_ref->instance())->tp_name;
}

bool PyInstance::hasAttr(const char* name) const
{
    return PyObject_HasAttrString(_ref->instance(), name) != 0;
}

PyInstance PyInstance::getAttr(const char* name) const
{
    const sp<PyInstanceRef> attr = sp<Stolen>::make(PyObject_GetAttrString(_ref->instance(), name));
    PythonInterpreter::instance()->referenceManager()->track(attr);
    return attr;
}

PyObject* PyInstance::call(PyObject* args) const
{
    try {
        return PyObject_Call(_ref->instance(), args, nullptr);
    }
    catch(const std::exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        Py_RETURN_NONE;
    }
}

bool PyInstance::isCallable() const
{
    return PyCallable_Check(_ref->instance()) != 0;
}

bool PyInstance::isNone() const
{
    return _ref != nullptr && _ref->instance() == Py_None;
}

PyObject* PyInstance::pyObject() const
{
    return _ref->instance();
}

void PyInstance::clear()
{
    _ref = nullptr;
}

PyInstance::Borrowed::Borrowed(PyObject* object)
    : PyInstanceRef(object)
{
}

void PyInstance::Borrowed::clear()
{
    DFATAL("You cannot clear a borrowed instance");
}

PyInstance::Owned::Owned(PyObject* object)
    : PyInstanceRef(object)
{
    Py_XINCREF(_instance);
}

PyInstance::Owned::~Owned()
{
    Py_XDECREF(_instance);
}

PyInstance::Stolen::Stolen(PyObject* object)
    : PyInstanceRef(object)
{
}

PyInstance::Stolen::~Stolen()
{
    Py_XDECREF(_instance);
}

}
}
}
