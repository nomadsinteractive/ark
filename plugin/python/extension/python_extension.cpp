#include "python/extension/python_extension.h"

#include "core/base/json.h"
#include "core/base/observer.h"
#include "core/base/scope.h"
#include "core/inf/variable.h"
#include "core/types/box.h"
#include "core/types/global.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "app/base/event.h"

#include "python/api.h"
#include "python/extension/python_extension.h"
#include "python/extension/py_instance_ref.h"
#include "python/extension/reference_manager.h"

#include "python/impl/adapter/collision_callback_python.h"
#include "python/impl/adapter/runnable_python.h"
#include "python/impl/adapter/event_listener_python.h"

namespace ark::plugin::python {

const sp<ReferenceManager>& PythonExtension::referenceManager() const
{
    return _reference_manager;
}

PythonExtension& PythonExtension::instance()
{
    Global<PythonExtension> instance;
    return static_cast<PythonExtension&>(instance);
}

PythonExtension::PythonExtension()
    : _reference_manager(sp<ReferenceManager>::make())
{
}

bool PythonExtension::isPyArkTypeObject(void* pyTypeObject) const
{
    return _type_by_py_object.find(pyTypeObject) != _type_by_py_object.end();
}

PyArkType* PythonExtension::getPyArkType(PyObject* pyObject)
{
    void* pyTypeObject = PyType_Check(pyObject) ? static_cast<void*>(pyObject) : static_cast<void*>(Py_TYPE(pyObject));
    auto iter = _type_by_py_object.find(pyTypeObject);
    return iter != _type_by_py_object.end() ? iter->second : nullptr;
}

PyArkType* PythonExtension::ensurePyArkType(PyObject* pyObject)
{
    DASSERT(pyObject);
    PyArkType* type = getPyArkType(pyObject);
    DCHECK(type, "Object \"%s\" is not a PyArkType instance", Py_TYPE(pyObject)->tp_name);
    return type;
}

PyObject* PythonExtension::toPyObject(const Box& box)
{
    if(!box)
        Py_RETURN_NONE;

    if(box.typeId() == Type<PyInstanceRef>::id())
    {
        PyObject* object = box.as<PyInstanceRef>()->instance();
        Py_XINCREF(object);
        return object;
    }

    const auto iter = _type_by_id.find(box.typeId());
    return iter != _type_by_id.end() ? iter->second->create(box) : getPyArkType<Box>()->create(box);
}

bool PythonExtension::isPyObject(TypeId type) const
{
    return (type == Type<PyInstance>::id()) || _type_by_id.find(type) != _type_by_id.end();
}

void PythonExtension::logErr() const
{
    PyErr_Print();
    PyObject* ferr = PySys_GetObject("stderr");
    PyObject* ret = ferr ? PyObject_CallMethod(ferr, "flush", nullptr) : nullptr;
    Py_XDECREF(ret);
    PyErr_Clear();
}

bool PythonExtension::exceptErr(PyObject* type) const
{
    PyObject* err = PyErr_Occurred();
    if(err)
    {
        if(PyErr_GivenExceptionMatches(err, type))
        {
            PyErr_Clear();
            return true;
        }
    }
    logErr();
    return false;
}

}
