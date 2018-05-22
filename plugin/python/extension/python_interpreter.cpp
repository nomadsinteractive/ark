#include "python/extension/python_interpreter.h"

#include "core/base/scope.h"
#include "core/inf/variable.h"
#include "core/inf/message_loop.h"
#include "core/types/box.h"
#include "core/types/global.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "graphics/base/color.h"
#include "graphics/base/vec3.h"

#include "app/base/event.h"

#include "python/api.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/py_garbage_collector.h"

#include "python/impl/adapter/collision_callback_python_adapter.h"
#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"

#include "python/impl/duck/py_callable_duck_type.h"
#include "python/impl/duck/py_numeric_duck_type.h"
#include "python/impl/duck/py_object_duck_type.h"

namespace ark {
namespace plugin {
namespace python {

sp<PythonInterpreter> PythonInterpreter::_INSTANCE;

sp<Runnable> PythonInterpreter::toRunnable(PyObject* object)
{
    if(isInstance<Runnable>(object))
        return toInstance<Runnable>(object);

    if(PyCallable_Check(object))
        return sp<PythonCallableRunnable>::make(sp<PyInstance>::make(PyInstance::adopt(object)));

    return asInterface<Runnable>(object);
}

sp<CollisionCallback> PythonInterpreter::toCollisionCallback(PyObject* object)
{
    return sp<CollisionCallbackPythonAdapter>::make(PyInstance::borrow(object));
}

sp<EventListener> PythonInterpreter::toEventListener(PyObject* object)
{
    if(PyCallable_Check(object))
        return sp<PythonCallableEventListener>::make(sp<PyInstance>::make(PyInstance::adopt(object)));

    return asInterface<EventListener>(object);
}

String PythonInterpreter::toString(PyObject* object, const char* encoding, const char* error)
{
    if(PyUnicode_Check(object))
        return unicodeToUTF8String(object, encoding, error);
    else if (PyBytes_Check(object))
        return PyBytes_AS_STRING(object);
    else
    {
        PyObject* str = PyObject_Str(object);
        const String r = unicodeToUTF8String(str, encoding, error);
        Py_DECREF(str);
        return r;
    }
    return "";
}

std::wstring PythonInterpreter::toWString(PyObject* object)
{
    if(PyUnicode_Check(object))
        return pyUnicodeToWString(object);
    else if (PyBytes_Check(object))
    {
        PyObject* unicode = PyUnicode_FromString(PyBytes_AsString(object));
        const std::wstring r = pyUnicodeToWString(unicode);
        Py_DECREF(unicode);
        return r;
    }
    else
    {
        PyObject* unicode = PyObject_Str(object);
        const std::wstring r = pyUnicodeToWString(unicode);
        Py_DECREF(unicode);
        return r;
    }
    return L"";
}

PyObject* PythonInterpreter::fromByteArray(const bytearray& bytes) const
{
    return PyBytes_FromStringAndSize(reinterpret_cast<const char*>(bytes->buf()), bytes->length());
}

sp<Numeric> PythonInterpreter::toNumeric(PyObject* object)
{
    if(PyLong_Check(object))
        return sp<Numeric::Impl>::make(static_cast<float>(PyLong_AsLong(object)));
    if(PyFloat_Check(object))
        return sp<Numeric::Impl>::make(static_cast<float>(PyFloat_AsDouble(object)));

    return asInterface<Numeric>(object);
}

sp<Integer> PythonInterpreter::toInteger(PyObject* object)
{
    if(PyLong_Check(object))
        return sp<Integer::Impl>::make(PyLong_AsLong(object));
    if(PyFloat_Check(object))
    {
        DWARN(false, "Casting from float to integer loses precision.");
        return sp<Integer::Impl>::make(static_cast<int32_t>(PyFloat_AsDouble(object)));
    }

    return asInterface<Integer>(object);
}

sp<Array<Color>> PythonInterpreter::toColorArray(PyObject* object)
{
    if(PyList_Check(object))
    {
        Py_ssize_t size = PyList_Size(object);
        sp<DynamicArray<Color>> colorArray = sp<DynamicArray<Color>>::make(size);
        Color* ptr = colorArray->buf();
        for(Py_ssize_t i = 0; i < size; i++)
        {
            uint32_t value = PyLong_AsUnsignedLong(PyList_GetItem(object, i));
            ptr[i] = Color(value);
        }
        return colorArray;
    }
    return nullptr;
}

sp<Scope> PythonInterpreter::toScope(PyObject* kws)
{
    if(kws)
    {
        PyObject* keys = PyDict_Keys(kws);
        const uint32_t size = PyList_Size(keys);
        const sp<Scope> scope = sp<Scope>::make();
        for(uint32_t i = 0; i < size; i ++)
        {
            PyObject* key = PyList_GetItem(keys, i);
            PyObject* item = PyDict_GetItem(kws, key);
            const String sKey = toString(key);
            if(PyFloat_Check(item) || PyLong_Check(item))
            {
                const sp<PyInstance> owned = sp<PyInstance>::make(PyInstance::adopt(item));
                sp<PyNumericDuckType> pyDuck = sp<PyNumericDuckType>::make(owned);
                scope->put<PyNumericDuckType>(sKey, pyDuck);
            }
            else if(PyBytes_Check(item) || PyUnicode_Check(item))
                scope->put<String>(sKey, sp<String>::make(toString(item)));
            else if(PyBool_Check(item))
                scope->put<Boolean>(sKey, sp<Boolean::Impl>::make(PyObject_IsTrue(item) != 0));
            else if(PyCallable_Check(item))
            {
                const sp<PyInstance> owned = sp<PyInstance>::make(PyInstance::adopt(item));
                sp<PyCallableDuckType> pyDuck = sp<PyCallableDuckType>::make(owned);
                pyDuck.absorb<PyGarbageCollector>(sp<PyGarbageCollector>::make(owned));
                scope->put<PyCallableDuckType>(sKey, pyDuck);
            }
            else if(isPyArkTypeObject(Py_TYPE(item)))
                scope->put(sKey, *reinterpret_cast<PyArkType::Instance*>(item)->box);
            else
            {
                const sp<PyInstance> owned = sp<PyInstance>::make(PyInstance::adopt(item));
                scope->put<PyObjectDuckType>(sKey, sp<PyObjectDuckType>::make(owned));
            }
        }
        Py_DECREF(keys);
        return scope;
    }
    return nullptr;
}

String PythonInterpreter::unicodeToUTF8String(PyObject* object, const char* encoding, const char* error)
{
    DCHECK(PyUnicode_Check(object), "Expecting an unicode object");
    return PyUnicode_AsUTF8(object);
}

std::wstring PythonInterpreter::pyUnicodeToWString(PyObject* unicode)
{
    PyUnicode_READY(unicode);
    uint32_t k = PyUnicode_KIND(unicode);
    void* data = PyUnicode_DATA(unicode);
    if(k == sizeof(wchar_t))
        return reinterpret_cast<wchar_t*>(data);
    std::vector<wchar_t> buf;
    if(k == 1)
        for(char* i = reinterpret_cast<char*>(data); *i; ++i)
            buf.push_back(static_cast<wchar_t>(*i));
    if(k == 2)
        for(uint16_t* i = reinterpret_cast<uint16_t*>(data); *i; ++i)
            buf.push_back(static_cast<wchar_t>(*i));
    if(k == 4)
        for(uint32_t* i = reinterpret_cast<uint32_t*>(data); *i; ++i)
            buf.push_back(static_cast<wchar_t>(*i));
    return std::wstring(buf.begin(), buf.end());
}

const sp<PythonInterpreter>& PythonInterpreter::instance()
{
    return _INSTANCE;
}

const sp<PythonInterpreter>& PythonInterpreter::newInstance()
{
    _INSTANCE = sp<PythonInterpreter>::make();
    return _INSTANCE;
}

bool PythonInterpreter::isPyArkTypeObject(void* pyTypeObject)
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
    NOT_NULL(pyObject);
    PyArkType* type = getPyArkType(pyObject);
    DCHECK(type, "Object \"%s\" is not a PyArkType instance", Py_TYPE(pyObject)->tp_name);
    return type;
}

PyObject* PythonInterpreter::toPyObject(const Box& box)
{
    if(!box)
        Py_RETURN_NONE;

    if(box.typeId() == Type<PyInstance>::id())
        return box.as<PyInstance>()->instance();

    auto iter = _type_by_id.find(box.typeId());
    DCHECK(iter != _type_by_id.end(), "Unknow box type");

    return iter->second->create(box);
}

void PythonInterpreter::logErr()
{
#ifdef ARK_FLAG_DEBUG
    PyErr_Print();
#else
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    const char* pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
    PyErr_Restore(ptype, pvalue, ptraceback);
    LOGE("%s", pStrErrorMessage);
#endif
    PyErr_Clear();
}

template<> ARK_PLUGIN_PYTHON_API String PythonInterpreter::toType<String>(PyObject* object)
{
    return toString(object);
}

template<> ARK_PLUGIN_PYTHON_API std::wstring PythonInterpreter::toType<std::wstring>(PyObject* object)
{
    return toWString(object);
}

template<> ARK_PLUGIN_PYTHON_API Box PythonInterpreter::toType<Box>(PyObject* object)
{
    return sp<PyInstance>::make(PyInstance::adopt(object)).pack();
}

template<> ARK_PLUGIN_PYTHON_API float PythonInterpreter::toType<float>(PyObject* object)
{
    DCHECK(PyNumber_Check(object), "Cannot cast Python object \"%s\" to float", object->ob_type->tp_name);
    return static_cast<float>(PyFloat_AsDouble(object));
}

template<> ARK_PLUGIN_PYTHON_API uint32_t PythonInterpreter::toType<uint32_t>(PyObject* object)
{
    if(isPyArkTypeObject(object))
    {
        PyArkType* pyArkType = PythonInterpreter::instance()->getPyArkType(object);
        DCHECK(pyArkType, "Cannot convert PyObject to PyArkType");
        return pyArkType->typeId();
    }
    return PyLong_AsUnsignedLong(object);
}

template<> ARK_PLUGIN_PYTHON_API int32_t PythonInterpreter::toType<int32_t>(PyObject* object)
{
    DCHECK(PyNumber_Check(object), "Cannot cast Python object \"%s\" to int32_t", object->ob_type->tp_name);
    return static_cast<int32_t>(PyLong_AsLong(object));
}

template<> ARK_PLUGIN_PYTHON_API V2 PythonInterpreter::toType<V2>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y;
        if(PyArg_ParseTuple(object, "ff", &x, &y))
            return V2(x, y);
    }
    if(PyList_Check(object))
    {
        uint32_t len = PyObject_Length(object);
        if(len == 2)
        {
            float x = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(object, 0)));
            float y = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(object, 1)));
            return V2(x, y);
        }
    }
    DFATAL("V2 object should be either length-2 tuple or list. (eg. [0, 0], (1.0, 1.0))");
    return V2();
}

template<> ARK_PLUGIN_PYTHON_API Color PythonInterpreter::toType<Color>(PyObject* object)
{
    if(PyLong_Check(object))
        return Color(static_cast<uint32_t>(PyLong_AsLongLong(object)));
    if(PyTuple_Check(object))
    {
        float r, g, b, a = 1.0f;
        if(PyArg_ParseTuple(object, "fff|f", &r, &g, &b, &a))
            return Color(r, g, b, a);
    }
    if(PyList_Check(object))
    {
        uint32_t len = PyObject_Length(object);
        if(len >= 3)
        {
            float r = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(object, 0)));
            float g = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(object, 1)));
            float b = static_cast<float>(PyFloat_AsDouble(PyList_GetItem(object, 2)));
            float a = len > 3 ? static_cast<float>(PyFloat_AsDouble(PyList_GetItem(object, 3))) : 1.0f;
            return Color(r, g, b, a);
        }
    }
    DFATAL("Color object should be either int or float array. (eg. 0xffffffff or (1.0, 1.0, 1.0, 1.0))");
    return Color();
}

//template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<Vec2>(const Vec2& value)
//{
//    return PythonInterpreter::instance()->pyNewObject<Vec2>(sp<Vec2>::make(value));
//}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<Vec3>(const Vec3& value)
{
    return PythonInterpreter::instance()->pyNewObject<Vec3>(sp<Vec3>::make(value));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<int32_t>(const int32_t& value)
{
    return PyLong_FromLong(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<uint32_t>(const uint32_t& value)
{
    return PyLong_FromUnsignedLong(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<uint8_t>(const uint8_t& value)
{
    return PyLong_FromLong(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<float>(const float& value)
{
    return PyFloat_FromDouble(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<Event::Action>(const Event::Action& value)
{
    return PyLong_FromLong(static_cast<long>(value));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<Event::Code>(const Event::Code& value)
{
    return PyLong_FromLong(static_cast<long>(value));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<Box>(const Box& value)
{
    return PythonInterpreter::instance()->toPyObject(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<String>(const String& value)
{
    return PyUnicode_FromString(value.c_str());
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<std::wstring>(const std::wstring& value)
{
    return PyUnicode_FromKindAndData(sizeof(wchar_t), value.c_str(), value.length());
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<bool>(const bool& value)
{
    return PyBool_FromLong(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<V2>(const V2& value)
{
    PyObject* v2 = PyTuple_New(2);
    PyTuple_SetItem(v2, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v2, 1, PyFloat_FromDouble(value.y()));
    return v2;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<V3>(const V3& value)
{
    PyObject* v3 = PyTuple_New(3);
    PyTuple_SetItem(v3, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v3, 1, PyFloat_FromDouble(value.y()));
    PyTuple_SetItem(v3, 2, PyFloat_FromDouble(value.z()));
    return v3;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::fromType<Color>(const Color& color)
{
    return PythonInterpreter::instance()->fromSharedPtr<Color>(sp<Color>::make(color));
}

}
}
}
