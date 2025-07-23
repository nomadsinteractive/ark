#include "python/extension/python_extension.h"

#include "core/base/json.h"
#include "core/base/observer.h"
#include "core/base/scope.h"
#include "core/types/box.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "python/extension/callable_v1.h"
#include "python/extension/py_instance_ref.h"
#include "python/extension/py_cast.h"
#include "python/extension/reference_manager.h"

#include "python/impl/adapter/runnable_python.h"

namespace ark::plugin::python {

namespace {

template<typename T> class CallableV1Trivial final : public CallableV1 {
public:
    CallableV1Trivial(std::function<void(const T&)> func)
        : _func(std::move(func)) {
    }

    void call(const PyInstance& pyobj) override
    {
        const T a1 = PyCast::ensureCppObject<T>(pyobj.pyObject());
        _func(a1);
    }

private:
    std::function<void(const T&)> _func;
};

}

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
    return _type_by_py_object.contains(pyTypeObject);
}

PyArkType* PythonExtension::getPyArkType(PyObject* pyObject)
{
    void* pyTypeObject = PyType_Check(pyObject) ? static_cast<void*>(pyObject) : static_cast<void*>(Py_TYPE(pyObject));
    const auto iter = _type_by_py_object.find(pyTypeObject);
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

    if(box.typeId() == Type<std::function<void(const V3&)>>::id())
    {
        std::function<void(const V3&)> func = *box.as<std::function<void(const V3&)>>();
        return pyNewObject(sp<CallableV1>::make<CallableV1Trivial<V3>>(std::move(func)));
    }

    const auto iter = _type_by_id.find(box.typeId());
    return iter != _type_by_id.end() ? iter->second->create(box) : getPyArkType<Box>()->create(box);
}

bool PythonExtension::isPyObject(const TypeId type) const
{
    return (type == Type<PyInstance>::id()) || _type_by_id.find(type) != _type_by_id.end();
}

void PythonExtension::printStack() const
{
    const PyInstance module = PyInstance::steal(PyImport_ImportModule("traceback"));
    const PyInstance callable = PyInstance::steal(PyObject_GetAttrString(module.pyObject(), "print_stack"));
    const PyInstance args = PyInstance::steal(PyTuple_New(0));
    const PyInstance kwargs = PyInstance::steal(PyDict_New());
    const PyInstance r = PyInstance::steal(PyObject_Call(callable.pyObject(), args.pyObject(), kwargs.pyObject()));
    flushErr();
}

void PythonExtension::logErr() const
{
    PyErr_Print();
    flushErr();
    PyErr_Clear();
}

void PythonExtension::flushErr() const
{
    PyObject* ferr = PySys_GetObject("stderr");
    PyObject* ret = ferr ? PyObject_CallMethod(ferr, "flush", nullptr) : nullptr;
    Py_XDECREF(ret);
}

bool PythonExtension::exceptErr(PyObject* type) const
{
    if(PyObject* err = PyErr_Occurred(); err && PyErr_GivenExceptionMatches(err, type))
    {
        PyErr_Clear();
        return true;
    }
    logErr();
    return false;
}

}
