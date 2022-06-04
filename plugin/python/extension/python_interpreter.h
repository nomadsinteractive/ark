#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_

#include <functional>
#include <map>
#include <type_traits>
#include <vector>

#include <Python.h>

#include "core/base/bean_factory.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/box.h"
#include "core/types/type.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

#include "app/forwarding.h"

#include "python/api.h"
#include "python/extension/py_ark_type.h"
#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

class ReferenceManager;

template <typename T> using remove_cvref_t = typename std::remove_reference<typename std::remove_cv<T>::value>::type;


class ARK_PLUGIN_PYTHON_API PythonInterpreter {
public:

    static const sp<PythonInterpreter>& instance();

    PythonInterpreter();

    sp<Numeric> toNumeric(PyObject* object, bool alert = true);
    sp<Boolean> toBoolean(PyObject* object, bool alert = true);
    String toString(PyObject* object, const char* encoding = nullptr, const char* error = nullptr) const;
    Scope toScope(PyObject* kws) const;

    template<typename T> PyObject* toPyObject(const T& obj) {
        return toPyObject_sfinae<T>(obj, nullptr);
    }

    template<typename T> T toCppObject(PyObject* obj) {
        return toCppObject_sfinae<T>(obj, nullptr);
    }

    template<typename T> sp<Array<sp<T>>> toSharedPtrArray(PyObject* object, bool alert) {
        DCHECK(PyList_Check(object), "Object \"%s\" is not a Python list", Py_TYPE(object)->tp_name);
        Py_ssize_t len = PyList_Size(object);
        std::vector<sp<T>> arr;
        for(Py_ssize_t i = 0; i < len; ++i) {
            PyObject* pyItem = PyList_GetItem(object, i);
            sp<T> inst = toSharedPtr<T>(pyItem, alert);
            if(!(pyItem == Py_None || inst || alert))
                return nullptr;
            arr.push_back(std::move(inst));
        }
        return sp<typename Array<sp<T>>::Vector>::make(std::move(arr));
    }

    PyObject* toPyObject_SharedPtr(const sp<PyInstanceRef>& inst);
    PyObject* toPyObject_SharedPtr(const sp<String>& inst);
    PyObject* toPyObject_SharedPtr(const bytearray& bytes);

    template<typename T> PyObject* toPyObject_SharedPtr(const sp<T>& object) {
        return toPyObject_SharedPtr_sfinae(object, nullptr);
    }

    template<typename T> PyObject* toPyObject_SharedPtr_sfinae(const sp<Array<T>>& arrayObject, typename std::enable_if<std::is_integral<T>::value>::type* ) {
        const uint32_t arrayLength = arrayObject->length();
        const T* arrayBuf = arrayObject->buf();
        PyObject* obj = PyList_New(arrayLength);
        for(uint32_t i = 0; i < arrayLength; ++i)
            PyList_SetItem(i, std::is_signed<T>::value ? PyLong_FromLong(arrayBuf[i]) : PyLong_FromUnsignedLong(arrayBuf[i]));
        return obj;
    }

    template<typename T> PyObject* toPyObject_SharedPtr_sfinae(const sp<Array<T>>& arrayObject, typename std::enable_if<std::is_floating_point<T>::value>::type* ) {
        const uint32_t arrayLength = arrayObject->length();
        const T* arrayBuf = arrayObject->buf();
        PyObject* obj = PyList_New(arrayLength);
        for(uint32_t i = 0; i < arrayLength; ++i)
            PyList_SetItem(i, PyFloat_FromDouble(arrayBuf[i]));
        return obj;
    }

    template<typename T> PyObject* toPyObject_SharedPtr_sfinae(const sp<T>& object, nullptr_t ) {
        if(!object)
            Py_RETURN_NONE;
        return pyNewObject<T>(object);
    }

    template<typename T> T toCppObject_impl(PyObject* object);
    template<typename T> PyObject* toPyObject_impl(const T& value);

    template<typename T> sp<T> toSharedPtr(PyObject* object, bool alert = true) {
        return toSharedPtrImpl<T>(object, alert);
    }

    template<typename T> int addPyArkType(PyArkType* pyArkType) {
        _type_by_id[Type<T>::id()] = pyArkType;
        _type_by_py_object[pyArkType->getPyTypeObject()] = pyArkType;
        return pyArkType->ready<T>();
    }

    template<typename T> PyArkType* getPyArkType() const {
        auto iter = _type_by_id.find(Type<T>::id());
        DCHECK(iter != _type_by_id.end(), "PyTypeObject not found");
        return reinterpret_cast<PyArkType*>(iter->second);
    }

    template<typename T> bool isInstance(PyObject* object) const {
        return PyObject_IsInstance(object, getPyArkType<T>()->getPyObject()) != 0;
    }

    template<typename T> PyObject* pyNewObject(const sp<T>& object) {
        TypeId typeId = object.ensureInterfaces()->typeId();
        return _type_by_id.find(typeId) != _type_by_id.end() ? toPyObject(object.ensureInterfaces()->as(object, typeId)) : getPyArkType<T>()->create(object);
    }

    template<typename T, typename P> void pyModuleAddType(PyObject* module, const char* moduleName, const char* typeName, PyTypeObject* base, long flags) {
        static T pyType(Strings::sprintf("%s.%s", moduleName, typeName), Strings::sprintf("%s.%s Type", moduleName, typeName), base, flags);
        int ret = addPyArkType<P>(&pyType);
        DCHECK(!ret, "PyArkType init failed");
        if(!ret)
            PyModule_AddObject(module, typeName, pyType.getPyObject());
    }

    bool isPyArkTypeObject(void* pyTypeObject) const;
    PyArkType* getPyArkType(PyObject* pyObject);
    PyArkType* ensurePyArkType(PyObject* pyObject);

    PyObject* toPyObject(const Box& box);
    bool isPyObject(TypeId type) const;
    bool isNoneOrNull(PyObject* pyObject) const;

    const sp<ReferenceManager>& referenceManager() const;

    void logErr() const;
    bool exceptErr(PyObject* type) const;

private:
    template<typename T> const sp<T> toSharedPtrImpl(PyObject* object, bool alert = true) {
        if(object == Py_None)
            return nullptr;

        PyTypeObject* pyType = reinterpret_cast<PyTypeObject*>(PyObject_Type(object));
        if(isPyArkTypeObject(pyType)) {
            PyArkType::Instance* instance = reinterpret_cast<PyArkType::Instance*>(object);
            const sp<T> s = instance->box->as<T>();
            DCHECK(!alert || s, "Casting \"%s\" to class \"%s\" failed", pyType->tp_name, Class::getClass<T>()->name());
            return s;
        }
        DCHECK(!alert, "Casting \"%s\" to class \"%s\" failed", pyType->tp_name, Class::getClass<T>()->name());
        return nullptr;
    }

    template<typename T> PyObject* fromIterable_sfinae(const T& list, typename std::remove_reference<decltype(list.front())>::type*) {
        PyObject* pyList = PyList_New(list.size());
        Py_ssize_t index = 0;
        for(const auto& i : list)
            PyList_SetItem(pyList, index++, toPyObject(i));
        return pyList;
    }

    template<typename T> PyObject* fromIterable_sfinae(const T& map, typename std::remove_reference<decltype(map.begin()->second)>::type*) {
        PyObject* pyDict = PyDict_New();
        for(const auto& i : map)
            PyDict_SetItem(pyDict, toPyObject(i.first), toPyObject(i.second));
        return pyDict;
    }

    template<typename T> PyObject* fromIterable_sfinae(const T& iterable, ...) {
        PyObject* pySet = PySet_New(0);
        for(const auto& i : iterable) {
            PyObject* obj = toPyObject(i);
            PySet_Add(pySet, obj);
            Py_XDECREF(obj);
        }
        return pySet;
    }

    template<typename T> T toCppObject_sfinae(PyObject* obj, typename T::_PtrType*) {
        return toSharedPtr<typename T::_PtrType>(obj, true);
    }
    template<typename T> T toCppObject_sfinae(PyObject* obj, decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))*) {
        typedef typename T::value_type U;
        return toCppCollectionObject_sfinae<T, U>(obj, nullptr);
    }
    template<typename T> T toCppObject_sfinae(PyObject* obj, typename std::enable_if<std::is_enum<T>::value>::type*) {
        return static_cast<T>(toCppObject_impl<int32_t>(obj));
    }
    template<typename... Args> PyObject* makeArgumentTuple(Args... args) {
        PyObject* tuple = PyTuple_New(sizeof...(Args));
        reduceArgumentTuple(tuple, 0, args...);
        return tuple;
    }
    template<typename T, typename... Args> void reduceArgumentTuple(PyObject *tuple, uint32_t idx, T arg, Args... args) {
        PyTuple_SetItem(tuple, idx, toPyObject(std::forward<T>(arg)));
        reduceArgumentTuple(tuple, idx + 1, args...);
    }
    void reduceArgumentTuple(PyObject *tuple, uint32_t idx) {
    }
    template<typename R, typename... Args> std::function<R(Args...)> toCppObject_function(PyObject* obj, std::function<R(Args...)>*) {
        PyInstance pyObj(PyInstance::own(obj));
        return [this, pyObj](Args... args) -> R {
            PyInstance tuple = PyInstance::steal(makeArgumentTuple<Args...>(args...));
            PyInstance result = PyInstance::steal(pyObj.call(tuple.pyObject()));
            if(result.isNullptr())
                logErr();
            return toCppObject<R>(result.pyObject());
        };
    }
    template<typename T> T toCppObject_sfinae(PyObject* obj, typename T::result_type*) {
        return toCppObject_function(obj, reinterpret_cast<T*>(0));
    }
    template<typename T> T toCppObject_sfinae(PyObject* obj, ...) {
        return toCppObject_impl<T>(obj);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& ptr, typename T::_PtrType*) {
        return toPyObject_SharedPtr(static_cast<sp<typename T::_PtrType>>(ptr));
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& iterable, typename std::enable_if<!(std::is_same<T, std::string>::value  || std::is_same<T, std::wstring>::value), decltype(iterable.begin())>::type*) {
        return fromIterable_sfinae<T>(iterable, nullptr);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, decltype(value.second)*) {
        PyObject* pyTuple = PyTuple_New(2);
        PyList_SetItem(pyTuple, 0, toPyObject(value.first));
        PyList_SetItem(pyTuple, 1, toPyObject(value.second));
        return pyTuple;
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, typename std::enable_if<std::is_enum<T>::value>::type*) {
        return PyLong_FromLong(static_cast<int32_t>(value));
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value && std::is_signed<T>::value>::type*) {
        return PyLong_FromLong(value);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value && std::is_unsigned<T>::value>::type*) {
        return PyLong_FromUnsignedLong(static_cast<uint32_t>(value));
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, typename std::enable_if<std::is_floating_point<T>::value>::type*) {
        return PyFloat_FromDouble(value);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, ...) {
        return toPyObject_impl<T>(value);
    }

    template<typename T, typename U> T toCppCollectionObject_sfinae(PyObject* obj, typename std::enable_if<std::is_same<T, std::string>::value || std::is_same<T, std::wstring>::value>::type*) {
        return toCppObject_impl<T>(obj);
    }
    template<typename T, typename U> T toCppCollectionObject_sfinae(PyObject* obj, ...) {
        T col;
        Py_ssize_t len = PyObject_Length(obj);
        DCHECK(len != -1, "Object \"%s\" has no length", Py_TYPE(obj)->tp_name);
        for(Py_ssize_t i = 0; i < len; ++i) {
            PyObject* key = PyLong_FromLong(static_cast<int32_t>(i));
            PyObject* item = PyObject_GetItem(obj, key);
            col.push_back(toCppObject<U>(item));
            Py_XDECREF(item);
            Py_DECREF(key);
        }
        return col;
    }

    sp<Vec2> toVec2(PyObject* object, bool alert);
    sp<Vec3> toVec3(PyObject* object, bool alert);
    sp<Integer> toInteger(PyObject* object, bool alert);
    sp<Runnable> toRunnable(PyObject* object, bool alert);
    sp<Observer> toObserver(PyObject* object, bool alert);
    sp<CollisionCallback> toCollisionCallback(PyObject* object);
    sp<EventListener> toEventListener(PyObject* object);

    String unicodeToUTF8String(PyObject* object, const char* encoding, const char* error) const;

    std::wstring pyUnicodeToWString(PyObject* unicode) const;
    std::wstring toWString(PyObject* object) const;

private:
    std::map<TypeId, PyArkType*> _type_by_id;
    std::map<void*, PyArkType*> _type_by_py_object;

    sp<ReferenceManager> _reference_manager;
};

template<> inline sp<String> PythonInterpreter::toSharedPtr<String>(PyObject* object, bool alert)
{
    return object == Py_None ? sp<String>::null() : sp<String>::make(toString(object));
}

template<> inline sp<Numeric> PythonInterpreter::toSharedPtr<Numeric>(PyObject* object, bool alert)
{
    return toNumeric(object, alert);
}

template<> inline sp<Integer> PythonInterpreter::toSharedPtr<Integer>(PyObject* object, bool alert)
{
    return toInteger(object, alert);
}

template<> inline sp<Boolean> PythonInterpreter::toSharedPtr<Boolean>(PyObject* object, bool alert)
{
    return toBoolean(object, alert);
}

template<> inline sp<Runnable> PythonInterpreter::toSharedPtr<Runnable>(PyObject* object, bool alert)
{
    return toRunnable(object, alert);
}

template<> inline sp<Observer> PythonInterpreter::toSharedPtr<Observer>(PyObject* object, bool alert)
{
    return toObserver(object, alert);
}

template<> inline sp<CollisionCallback> PythonInterpreter::toSharedPtr<CollisionCallback>(PyObject* object, bool alert)
{
    return toCollisionCallback(object);
}

template<> inline sp<EventListener> PythonInterpreter::toSharedPtr<EventListener>(PyObject* object, bool alert)
{
    return toEventListener(object);
}

template<> inline sp<Vec2> PythonInterpreter::toSharedPtr<Vec2>(PyObject* object, bool alert)
{
    return toVec2(object, alert);
}

template<> inline sp<Vec3> PythonInterpreter::toSharedPtr<Vec3>(PyObject* object, bool alert)
{
    return toVec3(object, alert);
}

}
}
}

#endif
