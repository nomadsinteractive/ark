#ifndef ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_
#define ARK_PLUGIN_PYTHON_EXTENSION_PYTHON_INTERPRETER_H_

#include <functional>
#include <map>
#include <type_traits>

#include <Python.h>

#include "core/base/bean_factory.h"
#include "core/forwarding.h"
#include "core/impl/array/dynamic_array.h"
#include "core/types/box.h"
#include "core/types/type.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

#include "python/api.h"
#include "python/extension/py_ark_type.h"

namespace ark {
namespace plugin {
namespace python {

class ARK_PLUGIN_PYTHON_API PythonInterpreter {
public:

    static const sp<PythonInterpreter>& instance();
    static const sp<PythonInterpreter>& newInstance();

    sp<Array<Color>> toColorArray(PyObject* object);
    sp<Numeric> toNumeric(PyObject* object);
    sp<Integer> toInteger(PyObject* object);
    sp<Runnable> toRunnable(PyObject* object);
    sp<CollisionCallback> toCollisionCallback(PyObject* object);
    sp<EventListener> toEventListener(PyObject* object);
    String toString(PyObject* object, const char* encoding = nullptr, const char* error = nullptr);
    std::wstring toWString(PyObject* object);
    sp<Scope> toScope(PyObject* kws);

    template<typename T> PyObject* toPyObject(const T& obj) {
        return toPyObject_sfinae<T>(obj, nullptr);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& ptr, typename T::_PtrType*) {
        return fromSharedPtr(ptr);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& iterable, decltype(iterable.begin())*) {
        return fromIterable_sfinae<T>(iterable, nullptr);
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, typename std::enable_if<std::is_enum<T>::value>::type*) {
        return fromType<int32_t>(static_cast<int32_t>(value));
    }
    template<typename T> PyObject* toPyObject_sfinae(const T& value, ...) {
        return fromType<T>(value);
    }

    template<typename T> T toCppObject(PyObject* obj) {
        return toCppObject_sfinae<T>(obj, nullptr);
    }
    template<typename T> T toCppObject_sfinae(PyObject* obj, typename T::_PtrType*) {
        return toSharedPtr<sp<typename T::_PtrType>>(obj);
    }
    template<typename T> T toCppObject_sfinae(PyObject* obj, ...) {
        return toType<T>(obj);
    }

    template<typename T> sp<Array<T>> toArray(PyObject* object) {
        DCHECK(PyList_Check(object), "Object \"\" is not a Python list", Py_TYPE(object)->tp_name);
        Py_ssize_t len = PyList_Size(object);
        const sp<Array<T>> arr = sp<DynamicArray<T>>::make(len);
        for(Py_ssize_t i = 0; i < len; ++i)
            arr->buf()[i] = toCppObject<T>(PyList_GetItem(object, i));
        return arr;
    }

    template<typename T> PyObject* fromSharedPtr(const sp<Array<T>>& array) {
        T* ptr = array->buf();
        uint32_t len = array->length();
        PyObject* pyList = PyList_New(len);
        for(uint32_t i = 0; i < len; i++)
            PyList_SetItem(pyList, i, toPyObject<T>(ptr[i]));
        return pyList;
    }

    PyObject* fromByteArray(const bytearray& bytes) const;

    template<typename T> T toType(PyObject* object);
    template<typename T> PyObject* fromType(const T& value);
    template<typename T> sp<T> toSharedPtr(PyObject* object) {
        if(object == Py_None)
            return nullptr;
        return asInterface<T>(object);
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

    template<typename T> PyArkType* getPyArkType() {
        auto iter = _type_by_id.find(Type<T>::id());
        DCHECK(iter != _type_by_id.end(), "PyTypeObject not found");
        return reinterpret_cast<PyArkType*>(iter->second);
    }

    template<typename T> bool isInstance(PyObject* object) {
        return PyObject_IsInstance(object, getPyArkType<T>()->getPyObject()) != 0;
    }

    template<typename T> const sp<T> asInterface(PyObject* object) {
        PyTypeObject* pyType = reinterpret_cast<PyTypeObject*>(PyObject_Type(object));
        if(isPyArkTypeObject(pyType)) {
            PyArkType::Instance* instance = reinterpret_cast<PyArkType::Instance*>(object);
            const sp<T> s = instance->box->as<T>();
            DCHECK(s, "Casting \"%s\" to class \"%s\" failed", pyType->tp_name, Class::getClass<T>()->name());
            return s;
        }
        return nullptr;
    }

    template<typename T> PyObject* pyNewObject(const sp<T>& object) {
        TypeId typeId = object.interfaces()->typeId();
        if(_type_by_id.find(typeId) != _type_by_id.end())
            return toPyObject(object.interfaces()->as(object.pack(), typeId));
        return getPyArkType<T>()->create(object.pack());
    }

    template<typename T, typename P> void pyModuleAddType(PyObject* module, const char* moduleName, const char* typeName, long flags) {
        static T pyType(Strings::sprintf("%s.%s", moduleName, typeName), Strings::sprintf("%s.%s Type", moduleName, typeName), flags);
        int ret = addPyArkType<P>(&pyType);
        DCHECK(!ret, "PyArkType init failed");
        if(!ret)
            PyModule_AddObject(module, typeName, pyType.getPyObject());
    }

    bool isPyArkTypeObject(void* pyTypeObject);
    PyArkType* getPyArkType(PyObject* pyObject);
    PyArkType* ensurePyArkType(PyObject* pyObject);

    PyObject* toPyObject(const Box& box);

    void logErr();

private:
    template<typename T> PyObject* fromIterable_sfinae(const T& iterable, decltype(iterable.at(0))*) {
        PyObject* pyList = PyList_New(0);
        for(const auto& i : iterable)
            PyList_Append(pyList, toPyObject(i));
        return pyList;
    }

    template<typename T> PyObject* fromIterable_sfinae(const T& iterable, ...) {
        PyObject* pySet = PySet_New(0);
        for(const auto& i : iterable)
            PySet_Add(pySet, toPyObject(i));
        return pySet;
    }

    template<typename T> const sp<T>& toInstance(PyObject* object) {
        PyArkType::Instance* instance = reinterpret_cast<PyArkType::Instance*>(object);
        return instance->unpack<T>();
    }

    String unicodeToUTF8String(PyObject* object, const char* encoding, const char* error);

    std::wstring pyUnicodeToWString(PyObject* unicode);


private:
    std::map<TypeId, PyArkType*> _type_by_id;
    std::map<void*, PyArkType*> _type_by_py_object;

    static sp<PythonInterpreter> _INSTANCE;
};

template<> inline sp<Numeric> PythonInterpreter::toSharedPtr<Numeric>(PyObject* object)
{
    return toNumeric(object);
}

template<> inline sp<Integer> PythonInterpreter::toSharedPtr<Integer>(PyObject* object)
{
    return toInteger(object);
}

template<> inline sp<Runnable> PythonInterpreter::toSharedPtr<Runnable>(PyObject* object)
{
    return toRunnable(object);
}

template<> inline sp<CollisionCallback> PythonInterpreter::toSharedPtr<CollisionCallback>(PyObject* object)
{
    return toCollisionCallback(object);
}

template<> inline sp<EventListener> PythonInterpreter::toSharedPtr<EventListener>(PyObject* object)
{
    return toEventListener(object);
}

template<> inline PyObject* PythonInterpreter::fromSharedPtr<Array<uint8_t>>(const bytearray& bytes)
{
    return fromByteArray(bytes);
}

}
}
}

#endif
