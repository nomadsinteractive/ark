#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_

#include <functional>
#include <map>
#include <type_traits>

#include <Python.h>

#include "core/base/bean_factory.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/types/box.h"
#include "core/types/type.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/api.h"
#include "python/extension/py_ark_type.h"
#include "python/extension/py_instance_ref.h"

namespace ark {
namespace plugin {
namespace python {

class ReferenceManager;

class ARK_PLUGIN_PYTHON_API PythonInterpreter {
public:

    static const sp<PythonInterpreter>& instance();

    PythonInterpreter();

    sp<Numeric> toNumeric(PyObject* object, bool alert = true);
    String toString(PyObject* object, const char* encoding = nullptr, const char* error = nullptr) const;
    Scope toScope(PyObject* kws) const;

    template<typename T> PyObject* toPyObject(const T& obj) {
        return toPyObject_sfinae<T>(obj, nullptr);
    }

    template<typename T> T toCppObject(PyObject* obj) {
        return toCppObject_sfinae<T>(obj, nullptr);
    }

    template<typename T> sp<Array<T>> toArray(PyObject* object) {
        DCHECK(PyList_Check(object), "Object \"\" is not a Python list", Py_TYPE(object)->tp_name);
        Py_ssize_t len = PyList_Size(object);
        const sp<Array<T>> arr = sp<typename Array<T>::Allocated>::make(len);
        for(Py_ssize_t i = 0; i < len; ++i)
            arr->buf()[i] = toCppObject<T>(PyList_GetItem(object, i));
        return arr;
    }

    template<typename T> PyObject* fromSharedPtr(const sp<Array<T>>& array) {
        T* ptr = array->buf();
        size_t len = array->length();
        PyObject* pyList = PyList_New(len);
        for(size_t i = 0; i < len; i++)
            PyList_SetItem(pyList, i, toPyObject<T>(ptr[i]));
        return pyList;
    }
    PyObject* fromSharedPtr(const sp<PyInstanceRef>& inst) {
        PyObject* obj = inst->instance();
        Py_XINCREF(obj);
        return obj;
    }
    PyObject* fromSharedPtr(const sp<String>& inst) {
        if(inst)
            return PyUnicode_FromString(inst->c_str());
        Py_RETURN_NONE;
    }

    PyObject* fromByteArray(const bytearray& bytes) const;

    template<typename T> T toType(PyObject* object);
    template<typename T> PyObject* fromType(const T& value);
    template<typename T> sp<T> toSharedPtr(PyObject* object, bool alert = true) {
        return asInterface<T>(object, alert);
    }
    template<typename T> PyObject* fromSharedPtr(const sp<T>& object) {
        if(!object)
            Py_RETURN_NONE;
        return pyNewObject<T>(object);
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

    template<typename T> const sp<T> asInterface(PyObject* object, bool alert = true) {
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
    template<typename T> PyObject* fromIterable_sfinae(const T& list, typename std::remove_reference<decltype(list.front())>::type*) {
        PyObject* pyList = PyList_New(list.size());
        Py_ssize_t index = 0;
        for(const auto& i : list)
            PyList_SetItem(pyList, index++, toPyObject(i));
        return pyList;
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
        return static_cast<T>(toType<int32_t>(obj));
    }
    template<typename T> T toCppObject_sfinae(PyObject* obj, ...) {
        return toType<T>(obj);
    }

    template<typename T> PyObject* toPyObject_sfinae(const T& ptr, typename T::_PtrType*) {
        return fromSharedPtr(static_cast<sp<typename T::_PtrType>>(ptr));
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& iterable, decltype(iterable.begin())*) {
        return fromIterable_sfinae<T>(iterable, nullptr);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, decltype(value.second)*) {
        PyObject* pyTuple = PyTuple_New(2);
        PyList_SetItem(pyTuple, 0, toPyObject(value.first));
        PyList_SetItem(pyTuple, 1, toPyObject(value.second));
        return pyTuple;
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, typename std::enable_if<std::is_enum<T>::value>::type*) {
        return fromType<int32_t>(static_cast<int32_t>(value));
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, ...) {
        return fromType<T>(value);
    }

    template<typename T, typename U> T toCppCollectionObject_sfinae(PyObject* obj, typename std::enable_if<std::is_same<T, std::string>::value || std::is_same<T, std::wstring>::value>::type*) {
        return toType<T>(obj);
    }
    template<typename T, typename U> T toCppCollectionObject_sfinae(PyObject* obj, ...) {
        T col;
        Py_ssize_t len = PyObject_Length(obj);
        DCHECK(len != -1, "Object \"%s\" has no length", Py_TYPE(obj)->tp_name);
        for(Py_ssize_t i = 0; i < len; ++i) {
            PyObject* key = PyLong_FromLong(i);
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

template<> inline sp<Numeric> PythonInterpreter::toSharedPtr<Numeric>(PyObject* object, bool alert)
{
    return toNumeric(object, alert);
}

template<> inline sp<Integer> PythonInterpreter::toSharedPtr<Integer>(PyObject* object, bool alert)
{
    return toInteger(object, alert);
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

template<> inline PyObject* PythonInterpreter::fromSharedPtr<Array<uint8_t>>(const bytearray& bytes)
{
    return fromByteArray(bytes);
}

}
}
}

#endif
