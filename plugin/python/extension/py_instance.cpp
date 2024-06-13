#include "python/extension/py_instance.h"

#include "core/types/shared_ptr.h"
#include "core/util/log.h"

#include "extension/python_interpreter.h"
#include "extension/reference_manager.h"

namespace ark::plugin::python {

PyInstance::PyInstance(sp<PyInstanceRef> ref)
    : _ref(std::move(ref))
{
}

PyInstance PyInstance::borrow(PyObject* object)
{
    return PyInstance(object ? sp<PyInstanceRef>::make(object, false) : nullptr);
}

PyInstance PyInstance::steal(PyObject* object)
{
    Py_XINCREF(object);
    return PyInstance(object ? sp<PyInstanceRef>::make(object, true) : nullptr);
}

PyInstance PyInstance::own(PyObject* object)
{
    Py_XINCREF(object);
    return PyInstance(object ? sp<PyInstanceRef>::make(object, true) : nullptr);
}

PyInstance PyInstance::track(PyObject* object)
{
    Py_XINCREF(object);
    sp<PyInstanceRef> ref = sp<PyInstanceRef>::make(object, true);
    PythonInterpreter::instance().referenceManager()->track(ref);
    return ref;
}

const sp<PyInstanceRef>& PyInstance::ref() const
{
    return _ref;
}

Box PyInstance::toBox() const
{
    return {_ref};
}

PyInstance::operator bool() const
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
    const sp<PyInstanceRef> attr = sp<PyInstanceRef>::make(PyObject_GetAttrString(_ref->instance(), name), true);
    PythonInterpreter::instance().referenceManager()->track(attr);
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

bool PyInstance::isList() const
{
    return PyList_Check(_ref->instance()) != 0;
}

bool PyInstance::isNone() const
{
    return _ref != nullptr && _ref->instance() == Py_None;
}

bool PyInstance::isNullptr() const
{
    return _ref == nullptr;
}

PyObject* PyInstance::pyObject() const
{
    return _ref->instance();
}

}
