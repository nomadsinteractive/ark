#pragma once

#include <array>
#include <functional>
#include <map>
#include <optional>
#include <type_traits>
#include <vector>

#include <Python.h>

#include "core/base/bean_factory.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/box.h"
#include "core/types/optional.h"
#include "core/types/type.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

#include "app/forwarding.h"

#include "python/api.h"
#include "python/extension/py_instance.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/py_bridge.h"

namespace ark::plugin::python {

class ARK_PLUGIN_PYTHON_API PyCast {
public:
    static Optional<sp<StringVar>> toStringVar(PyObject* object);

    static Optional<sp<Numeric>> toNumeric(PyObject* object, bool alert = true);
    static Optional<sp<Boolean>> toBoolean(PyObject* object, bool alert = true);
    static Optional<String> toStringExact(PyObject* object, const char* encoding = nullptr, const char* error = nullptr);
    static String toString(PyObject* object, const char* encoding = nullptr, const char* error = nullptr);
    static Scope toScope(PyObject* kws);
    static Traits toTraits(PyObject* args, size_t offset);

    template<typename T> static PyObject* toPyObject(const T& obj) {
        return toPyObject_sfinae<T>(obj, nullptr);
    }

    template<typename T> static Optional<T> toCppObject(PyObject* obj) {
        return toCppObject_sfinae<T>(obj, nullptr);
    }

    template<typename T> static T ensureCppObject(PyObject* obj) {
        Optional<T> opt = toCppObject_sfinae<T>(obj, nullptr);
        CHECK(opt, "Casting \"%s\" to class \"%s\" failed", Py_TYPE(obj)->tp_name, Class::getClass<T>()->name());
        return opt.value();
    }

    static PyObject* toPyObject_SharedPtr(const sp<PyInstanceRef>& inst);
    static PyObject* toPyObject_SharedPtr(const sp<String>& inst);

    template<typename T> static PyObject* toPyObject_SharedPtr(const sp<T>& object) {
        return toPyObject_SharedPtr_sfinae(object, nullptr);
    }

    template<typename T> static PyObject* toPyObject_SharedPtr_sfinae(const sp<T>& object, std::nullptr_t ) {
        if(!object)
            return PyBridge::incRefNone();
        return PythonInterpreter::instance().pyNewObject<T>(object);
    }

    template<typename T> static Optional<T> toCppObject_impl(PyObject* object);
    template<typename T> static PyObject* toPyObject_impl(const T& value);

    template<typename T> static Optional<sp<T>> toSharedPtr(PyObject* object, bool alert = true) {
        if(PyBridge::isPyNone(object))
            return sp<T>();
        return toSharedPtrImpl<T>(object, alert);
    }

    template<typename T> static sp<T> toSharedPtrOrNull(PyObject* object) {
        if(PyBridge::isPyNone(object))
            return nullptr;
        Optional<sp<T>> opt = toSharedPtrDefault<T>(object);
        return opt ? opt.value() : nullptr;
    }

    template<typename T> static sp<T> ensureSharedPtr(PyObject* object, bool alert = true) {
        Optional<sp<T>> opt = toSharedPtr<T>(object, alert);
        CHECK(opt, "Casting \"%s\" to class \"%s\" failed", Py_TYPE(object)->tp_name, Class::getClass<T>()->name());
        return opt.value();
    }

    template<> Optional<long> toCppObject_impl<long>(PyObject* object) {
        if(PyLong_Check(object))
            return static_cast<long>(PyLong_AsLong(object));
        return {};
    }

    static PyObject* toPyObject(const Box& box);

    static bool isNoneOrNull(PyObject* pyObject);

    template<typename F, F f> struct func_wrapper_impl;
    template<typename R, typename... Args, R(*f)(Args...)> struct func_wrapper_impl<R(*)(Args...), f> {
        static R wrapped(Args... args) {
            try {
                return f(args...);
            }
            catch(const std::exception& e) {
                PyBridge::setRuntimeErrString(e.what());
            }
            return (R)(0);
        }
    };

    template<typename F, F f> constexpr static auto RuntimeFuncWrapper = func_wrapper_impl<F, f>::wrapped;

private:
    template<typename T> static Optional<sp<T>> toSharedPtrImpl(PyObject* object, bool alert = true) {
        return toSharedPtrDefault<T>(object);
    }

    template<typename T> static Optional<sp<T>> toSharedPtrDefault(PyObject* object) {
        if(PyBridge::isPyNone(object))
            return Optional<sp<T>>(sp<T>());

        PyTypeObject* pyType = reinterpret_cast<PyTypeObject*>(PyBridge::PyObject_Type(object));
        if(PythonInterpreter::instance().isPyArkTypeObject(pyType)) {
            PyArkType::Instance* instance = reinterpret_cast<PyArkType::Instance*>(object);
            DASSERT(instance->box);
            sp<T> s = instance->box->as<T>();
            return s ? Optional<sp<T>>(std::move(s)) : Optional<sp<T>>();
        }
        return Optional<sp<T>>();
    }

    template<typename T> static PyObject* fromIterable_sfinae(const T& list, typename std::remove_reference<decltype(list.front())>::type*) {
        constexpr bool isArray = std::is_same_v<T, std::array<typename T::value_type, sizeof(T) / sizeof(typename T::value_type)>>;
        PyObject* pyList;
        if constexpr(isArray)
            pyList = PyBridge::PyTuple_New(list.size());
        else
            pyList = PyBridge::PyList_New(list.size());
        Py_ssize_t index = 0;
        for(const auto& i : list)
            if constexpr(isArray)
                PyBridge::PyTuple_SetItem(pyList, index++, toPyObject(i));
            else
                PyBridge::PyList_SetItem(pyList, index++, toPyObject(i));
        return pyList;
    }

    template<typename T> static PyObject* fromIterable_sfinae(const T& map, typename std::remove_reference<decltype(map.begin()->second)>::type*) {
        PyObject* pyDict = PyDict_New();
        for(const auto& i : map)
            PyBridge::PyDict_SetItem(pyDict, toPyObject(i.first), toPyObject(i.second));
        return pyDict;
    }

    template<typename T> static PyObject* fromIterable_sfinae(const T& iterable, ...) {
        PyObject* pySet = PySet_New(0);
        for(const auto& i : iterable) {
            PyObject* obj = toPyObject(i);
            PySet_Add(pySet, obj);
            Py_XDECREF(obj);
        }
        return pySet;
    }

    template<typename T> static Optional<T> toCppObject_sfinae(PyObject* obj, typename T::_PtrType*) {
        return toSharedPtr<typename T::_PtrType>(obj, true);
    }
    template<typename T> static Optional<T> toCppObject_sfinae(PyObject* obj, typename T::value_type*) {
        return toCppCollectionObject_sfinae<T, typename T::value_type>(obj, nullptr);
    }
    template<typename T> static Optional<T> toCppObject_sfinae(PyObject* obj, std::enable_if_t<std::is_enum_v<T>>*) {
        PyInstance pyObj(PyInstance::steal(PyBridge::PyNumber_Index(obj)));
        if(pyObj) {
            return Optional<T>(static_cast<T>(PyBridge::PyLong_AsLong(pyObj.pyObject())));
        }
        return Optional<T>();
    }
    template<typename... Args> static PyObject* makeArgumentTuple(Args... args) {
        PyObject* tuple = PyTuple_New(sizeof...(Args));
        reduceArgumentTuple(tuple, 0, args...);
        return tuple;
    }
    template<typename T, typename... Args> static void reduceArgumentTuple(PyObject *tuple, uint32_t idx, T arg, Args... args) {
        PyTuple_SetItem(tuple, idx, toPyObject(std::forward<T>(arg)));
        reduceArgumentTuple(tuple, idx + 1, args...);
    }
    static void reduceArgumentTuple(PyObject *tuple, uint32_t idx) {
    }
    template<typename R, typename... Args> static Optional<std::function<R(Args...)>> toCppObject_function(PyObject* obj, std::function<R(Args...)>*) {
        if(!PyCallable_Check(obj))
            return Optional<std::function<R(Args...)>>();
        PyInstance pyObj(PyInstance::own(obj));
        return Optional<std::function<R(Args...)>>([pyObj](Args... args) -> R {
            PyInstance tuple = PyInstance::steal(makeArgumentTuple<Args...>(args...));
            PyInstance result = PyInstance::steal(pyObj.call(tuple.pyObject()));
            if(result.isNullptr())
                PythonInterpreter::instance().logErr();
            if constexpr(std::is_same_v<R, void>)
                return;
            else
                return ensureCppObject<R>(result.pyObject());
        });
    }
    template<typename T> static Optional<T> toCppObject_sfinae(PyObject* obj, typename T::result_type*) {
        return toCppObject_function(obj, reinterpret_cast<T*>(0));
    }
    template<typename T> static Optional<T> toCppObject_sfinae(PyObject* obj, ...) {
        return toCppObject_impl<T>(obj);
    }
    template<typename T> static PyObject* toPyObject_sfinae(const T& ptr, typename T::_PtrType*) {
        return toPyObject_SharedPtr(static_cast<sp<typename T::_PtrType>>(ptr));
    }
    template<typename T> static PyObject* toPyObject_sfinae(const T& ptr, typename T::OPT_TYPE*) {
        return ptr ? toPyObject(ptr.value()) : PyBridge::incRefNone();
    }
    template<typename T> static PyObject* toPyObject_sfinae(const T& iterable, std::enable_if_t<!(std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> || std::is_same_v<T, Span>), decltype(iterable.begin())>*) {
        return fromIterable_sfinae<T>(iterable, nullptr);
    }
    template<typename T> static PyObject* toPyObject_sfinae(const T& value, std::enable_if_t<std::is_enum<T>::value>*) {
        return PyBridge::PyLong_FromLong(static_cast<int32_t>(value));
    }
    template<typename T> static PyObject* toPyObject_sfinae(const T& value, std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<T, bool>>*) {
        if constexpr(sizeof(T) <= sizeof(int32_t)) {
            if constexpr(std::is_signed_v<T>)
                return PyBridge::PyLong_FromLong(value);
            else
                return PyBridge::PyLong_FromUnsignedLong(static_cast<uint32_t>(value));
        } else {
            static_assert(sizeof(T) == sizeof(int64_t));
            if constexpr(std::is_signed_v<T>)
                return PyBridge::PyLong_FromLongLong(value);
            else
                return PyBridge::PyLong_FromUnsignedLongLong(static_cast<uint64_t>(value));
        }
    }
    template<typename T> static PyObject* toPyObject_sfinae(const T& value, std::enable_if_t<std::is_floating_point<T>::value>*) {
        return PyBridge::PyFloat_FromDouble(value);
    }
    template<typename T> static PyObject* toPyObject_sfinae(const T& value, ...) {
        return toPyObject_impl<T>(value);
    }

    template<typename T, typename U> static Optional<T> toCppCollectionObject_sfinae(PyObject* obj, std::pair<typename T::key_type, typename T::mapped_type>*) {
        return toCppObject_map<T, typename T::key_type, typename T::mapped_type>(obj);
    }
    template<typename T, typename U> static Optional<T> toCppCollectionObject_sfinae(PyObject* obj, std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>>*) {
        return toCppObject_impl<T>(obj);
    }
    template<typename T, typename U> static Optional<T> toCppCollectionObject_sfinae(PyObject* obj, std::enable_if_t<!std::is_same_v<T, std::string> && !std::is_same_v<T, std::wstring>, decltype(&T::push_back)>*) {
        Py_ssize_t len = !PyBridge::isPyDictExact(obj) ? PyBridge::PyObject_Size(obj) : -1;
        if(len == -1)
            return Optional<T>();
        T col;
        if(copyToCppObject<U>(obj, std::back_inserter(col)))
            return col;
        return Optional<T>();
    }
    template<typename T, typename U> static Optional<T> toCppCollectionObject_sfinae(PyObject* obj, std::enable_if_t<std::is_same_v<T, typename std::array<U, sizeof(T) / sizeof(U)>>>*) {
        Py_ssize_t len = !PyBridge::isPyDictExact(obj) ? PyBridge::PyObject_Size(obj) : -1;
        if(len != sizeof(T) / sizeof(U))
            return Optional<T>();
        std::array<U, sizeof(T) / sizeof(U)> array;
        if(copyToCppObject<U>(obj, array.begin()))
            return array;
        return Optional<T>();
    }
    template<typename T, typename U> static Optional<T> toCppCollectionObject_sfinae(PyObject* obj, ...) {
        Py_ssize_t len = !PyBridge::isPyDictExact(obj) ? PyBridge::PyObject_Size(obj) : -1;
        if(len == -1)
            return Optional<T>();
        T col;
        if(copyToCppObject<U>(obj, std::inserter(col, col.begin())))
            return col;
        return Optional<T>();
    }
    template<typename T, typename OutIt> static bool copyToCppObject(PyObject* obj, OutIt outIter) {
        PyObject* iter = PyBridge::PyObject_GetIter(obj);
        ASSERT(iter);
        PyObject* next = PyBridge::PyIter_Next(iter);
        bool successed = true;
        while(next != nullptr) {
            Optional<T> opt = toCppObject<T>(next);
            PyBridge::decRef(next);
            if(!opt) {
                successed = false;
                break;
            }
            next = PyBridge::PyIter_Next(iter);
            *outIter = std::move(opt.value());
            ++outIter;
        }
        PyBridge::decRef(iter);
        return successed;
    }
    template<typename T, typename K, typename V> static Optional<T> toCppObject_map(PyObject* obj) {
        if(!PyBridge::isPyDictExact(obj))
            return Optional<T>();

        T map;
        PyInstance keys(PyInstance::steal(PyBridge::PyDict_Keys(obj)));
        Py_ssize_t keySize = PyBridge::PyList_Size(keys.pyObject());
        for(Py_ssize_t i = 0; i < keySize; ++i) {
            PyObject* key = PyBridge::PyList_GetItem(keys.pyObject(), i);
            PyObject* item = PyBridge::PyDict_GetItem(obj, key);
            Optional<K> optKey = toCppObject<K>(key);
            Optional<V> optValue = toCppObject<V>(item);
            if(!optKey || !optValue)
                return Optional<T>();
            map[optKey.value()] = optValue.value();
        }
        return map;
    }

    static sp<Vec2> toVec2(PyObject* object, bool alert);
    static sp<Vec3> toVec3(PyObject* object, bool alert);
    static Optional<sp<Integer>> toInteger(PyObject* object, bool alert);
    static Optional<sp<Runnable>> toRunnable(PyObject* object, bool alert);
    static Optional<sp<Observer>> toObserver(PyObject* object, bool alert);
    static sp<CollisionCallback> toCollisionCallback(PyObject* object);
    static sp<EventListener> toEventListener(PyObject* object);

    static String unicodeToUTF8String(PyObject* object, const char* encoding, const char* error);

    static std::wstring pyUnicodeToWString(PyObject* unicode);
    static std::wstring toWString(PyObject* object);

};

template<> inline Optional<sp<String>> PyCast::toSharedPtrImpl<String>(PyObject* object, bool alert)
{
    Optional<String> opt = toStringExact(object);
    if(opt)
        return sp<String>::make(std::move(opt.value()));
    return Optional<sp<String>>();
}

template<> inline Optional<sp<StringVar>> PyCast::toSharedPtrImpl<StringVar>(PyObject* object, bool alert)
{
    return toStringVar(object);
}

template<> inline Optional<sp<Numeric>> PyCast::toSharedPtrImpl<Numeric>(PyObject* object, bool alert)
{
    return toNumeric(object, alert);
}

template<> inline Optional<sp<Integer>> PyCast::toSharedPtrImpl<Integer>(PyObject* object, bool alert)
{
    return toInteger(object, alert);
}

template<> inline Optional<sp<Boolean>> PyCast::toSharedPtrImpl<Boolean>(PyObject* object, bool alert)
{
    return toBoolean(object, alert);
}

template<> inline Optional<sp<Runnable>> PyCast::toSharedPtrImpl<Runnable>(PyObject* object, bool alert)
{
    return toRunnable(object, alert);
}

template<> inline Optional<sp<Observer>> PyCast::toSharedPtrImpl<Observer>(PyObject* object, bool alert)
{
    return toObserver(object, alert);
}

template<> inline Optional<sp<CollisionCallback>> PyCast::toSharedPtrImpl<CollisionCallback>(PyObject* object, bool alert)
{
    return toCollisionCallback(object);
}

template<> inline Optional<sp<EventListener>> PyCast::toSharedPtrImpl<EventListener>(PyObject* object, bool alert)
{
    return toEventListener(object);
}

template<> inline Optional<sp<Vec2>> PyCast::toSharedPtrImpl<Vec2>(PyObject* object, bool alert)
{
    sp<Vec2> vec2 = toVec2(object, false);
    return vec2 ? Optional<sp<Vec2>>(std::move(vec2)) : Optional<sp<Vec2>>();
}

template<> inline Optional<sp<Vec3>> PyCast::toSharedPtrImpl<Vec3>(PyObject* object, bool alert)
{
    sp<Vec3> vec3 = toVec3(object, false);
    return vec3 ? Optional<sp<Vec3>>(std::move(vec3)) : Optional<sp<Vec3>>();
}

template<> inline Optional<sp<ByteArray>> PyCast::toSharedPtrImpl<ByteArray>(PyObject* object, bool alert) {
    if(PyBytes_Check(object)) {
        Py_ssize_t len = PyBytes_Size(object);
        return sp<ByteArray>::make<ByteArray::Borrowed>(reinterpret_cast<uint8_t*>(PyBytes_AsString(object)), len);
    }
    if(PyObject_CheckBuffer(object)) {
        Py_buffer buf;
        PyObject_GetBuffer(object, &buf, PyBUF_C_CONTIGUOUS);
        sp<ByteArray> arr = sp<ByteArray::Allocated>::make(buf.len);
        memcpy(arr->buf(), buf.buf, buf.len);
        PyBuffer_Release(&buf);
        return arr;
    }
    return toSharedPtrDefault<ByteArray>(object);
}

}
