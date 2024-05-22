#include "python/extension/python_interpreter.h"

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
#include "python/extension/python_interpreter.h"
#include "python/extension/py_instance_ref.h"
#include "python/extension/reference_manager.h"

#include "python/impl/adapter/collision_callback_python_adapter.h"
#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"

#include "python/impl/duck/py_callable_duck_type.h"
#include "python/impl/duck/py_numeric_duck_type.h"
#include "python/impl/duck/py_object_duck_type.h"
#include "python/impl/duck/py_vec_duck_type.h"

namespace ark::plugin::python {

const sp<ReferenceManager>& PythonInterpreter::referenceManager() const
{
    return _reference_manager;
}

PythonInterpreter& PythonInterpreter::instance()
{
    Global<PythonInterpreter> instance;
    return static_cast<PythonInterpreter&>(instance);
}

PythonInterpreter::PythonInterpreter()
    : _reference_manager(sp<ReferenceManager>::make())
{
}

bool PythonInterpreter::isPyArkTypeObject(void* pyTypeObject) const
{
    return _type_by_py_object.find(pyTypeObject) != _type_by_py_object.end();
}

PyArkType* PythonInterpreter::getPyArkType(PyObject* pyObject)
{
    void* pyTypeObject = PyType_Check(pyObject) ? static_cast<void*>(pyObject) : static_cast<void*>(Py_TYPE(pyObject));
    auto iter = _type_by_py_object.find(pyTypeObject);
    return iter != _type_by_py_object.end() ? iter->second : nullptr;
}

PyArkType* PythonInterpreter::ensurePyArkType(PyObject* pyObject)
{
    DASSERT(pyObject);
    PyArkType* type = getPyArkType(pyObject);
    DCHECK(type, "Object \"%s\" is not a PyArkType instance", Py_TYPE(pyObject)->tp_name);
    return type;
}

PyObject* PythonInterpreter::toPyObject(const Box& box)
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

bool PythonInterpreter::isPyObject(TypeId type) const
{
    return (type == Type<PyInstance>::id()) || _type_by_id.find(type) != _type_by_id.end();
}

void PythonInterpreter::logErr() const
{
    PyErr_Print();
    PyObject* ferr = PySys_GetObject("stderr");
    PyObject* ret = ferr ? PyObject_CallMethod(ferr, "flush", nullptr) : nullptr;
    Py_XDECREF(ret);
    PyErr_Clear();
}

bool PythonInterpreter::exceptErr(PyObject* type) const
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
