#include "python/extension/python_extension.h"

#include "core/base/json.h"
#include "core/base/observer.h"
#include "core/base/scope.h"
#include "core/types/box.h"
#include "core/types/global.h"
#include "core/util/log.h"

#include "app/base/searching_node.h"

#include "python/extension/callable_v1.h"
#include "python/extension/py_instance_ref.h"
#include "python/extension/py_cast.h"
#include "python/extension/reference_manager.h"

#include "python/impl/adapter/runnable_python.h"

namespace ark::plugin::python {

namespace {

template<typename T> Optional<T> getArgumentOpt(const Traits& args, const size_t argidx)
{
    if(argidx >= args.table().size())
        return {};
    return PyCast::toCppObject<T>(args.table().values().at(argidx).as<PyInstanceRef>()->instance());
}

class CallableA1SearchingNode final : public CallableV1 {
public:
    CallableA1SearchingNode(std::function<void(const SearchingNode&)> func)
        : _func(std::move(func)) {
    }

    void call(const Traits& args) override
    {
        ASSERT(args.table().size() >= 1);
        const auto a1 = PyCast::ensureCppObject<V3>(args.table().values().at(0).as<PyInstanceRef>()->instance());
        _func({a1, getArgumentOpt<bool>(args, 1), getArgumentOpt<float>(args, 2)});
    }

private:
    std::function<void(const SearchingNode&)> _func;
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

    typedef std::function<void(const SearchingNode&)> FuncType;
    if(box.typeId() == Type<FuncType>::id())
    {
        FuncType func = *box.as<FuncType>();
        return pyNewObject(sp<CallableV1>::make<CallableA1SearchingNode>(std::move(func)));
    }

    const auto iter = _type_by_id.find(box.typeId());
    return iter != _type_by_id.end() ? iter->second->create(box) : getPyArkType<Box>()->create(box);
}

bool PythonExtension::isPyObject(const TypeId type) const
{
    return (type == Type<PyInstance>::id()) || _type_by_id.contains(type);
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

PythonExtension::EnsureGIL::EnsureGIL(PythonExtension* extension)
    : _extension(extension->_thread_state ? extension : nullptr)
{
    if(_extension)
    {
        PyEval_RestoreThread(_extension->_thread_state);
        _extension->_thread_state = nullptr;
    }
}

PythonExtension::EnsureGIL::EnsureGIL(EnsureGIL&& other)
    : _extension(other._extension)
{
    other._extension = nullptr;
}

PythonExtension::EnsureGIL::~EnsureGIL()
{
    if(_extension)
        _extension->_thread_state = PyEval_SaveThread();
}

PythonExtension::EnsureGIL PythonExtension::ensureGIL()
{
    return EnsureGIL(this);
}

}
