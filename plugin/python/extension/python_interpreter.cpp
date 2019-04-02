#include "python/extension/python_interpreter.h"

#include "core/base/scope.h"
#include "core/inf/variable.h"
#include "core/inf/message_loop.h"
#include "core/types/box.h"
#include "core/types/global.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "graphics/base/color.h"
#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/util/vec2_util.h"

#include "app/base/event.h"

#include "python/api.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/py_garbage_collector.h"
#include "python/extension/reference_manager.h"

#include "python/impl/adapter/collision_callback_python_adapter.h"
#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"

#include "python/impl/duck/py_callable_duck_type.h"
#include "python/impl/duck/py_numeric_duck_type.h"
#include "python/impl/duck/py_object_duck_type.h"

namespace ark {
namespace plugin {
namespace python {

namespace {

class PyNumeric : public Numeric {
public:
    PyNumeric(PyObject* attr)
        : _attr(PyInstance::steal(attr)) {
    }

    virtual float val() override {
        return static_cast<float>(PyFloat_AsDouble(_attr.object()));
    }

private:
    PyInstance _attr;
};

}

sp<Runnable> PythonInterpreter::toRunnable(PyObject* object)
{
    if(isInstance<Runnable>(object))
        return toInstance<Runnable>(object);

    if(PyCallable_Check(object))
        return sp<PythonCallableRunnable>::make(PyInstance::track(object));

    return asInterface<Runnable>(object);
}

sp<CollisionCallback> PythonInterpreter::toCollisionCallback(PyObject* object)
{
    return sp<CollisionCallbackPythonAdapter>::make(PyInstance::borrow(object));
}

sp<EventListener> PythonInterpreter::toEventListener(PyObject* object)
{
    if(PyCallable_Check(object))
        return sp<PythonCallableEventListener>::make(PyInstance::track(object));

    return asInterface<EventListener>(object);
}

String PythonInterpreter::toString(PyObject* object, const char* encoding, const char* error)
{
    if(object)
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

sp<Vec2> PythonInterpreter::toVec2(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        PyObject* x, *y;
        if(PyArg_ParseTuple(object, "OO", &x, &y))
            return Vec2Util::create(toNumeric(x), toNumeric(y));
    }
    return asInterface<Vec2>(object);
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

    sp<Numeric> arkInstance = asInterface<Numeric>(object);
    if(arkInstance)
        return arkInstance;

    PyObject* val = PyObject_GetAttrString(object, "val");
    if(val != Py_None)
        return sp<PyNumeric>::make(val);
    Py_XDECREF(val);
    return nullptr;
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

sp<Scope> PythonInterpreter::toScope(PyObject* kws)
{
    if(kws)
    {
        PyObject* keys = PyDict_Keys(kws);
        const Py_ssize_t size = PyList_Size(keys);
        const sp<Scope> scope = sp<Scope>::make();
        for(Py_ssize_t i = 0; i < size; i ++)
        {
            PyObject* key = PyList_GetItem(keys, i);
            PyObject* item = PyDict_GetItem(kws, key);
            const String sKey = toString(key);
            if(isPyArkTypeObject(Py_TYPE(item)))
                scope->put(sKey, *reinterpret_cast<PyArkType::Instance*>(item)->box);
            else if(PyBool_Check(item))
                scope->put<Boolean>(sKey, sp<Boolean::Const>::make(PyObject_IsTrue(item) != 0));
            else if(PyFloat_Check(item) || PyLong_Check(item) || PyObject_HasAttrString(item, "val"))
            {
                const sp<PyInstance> owned = PyInstance::track(item);
                sp<PyNumericDuckType> pyDuck = sp<PyNumericDuckType>::make(owned);
                scope->put<PyNumericDuckType>(sKey, pyDuck);
            }
            else if(PyBytes_Check(item) || PyUnicode_Check(item))
                scope->put<String>(sKey, sp<String>::make(toString(item)));
            else if(PyCallable_Check(item))
            {
                const sp<PyInstance> owned = PyInstance::track(item);
                sp<PyCallableDuckType> pyDuck = sp<PyCallableDuckType>::make(owned);
                pyDuck.absorb<PyGarbageCollector>(sp<PyGarbageCollectorImpl>::make(owned));
                scope->put<PyCallableDuckType>(sKey, pyDuck);
            }
            else
            {
                const sp<PyInstance> owned = PyInstance::track(item);
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

const sp<ReferenceManager>& PythonInterpreter::referenceManager() const
{
    return _reference_manager;
}

const sp<PythonInterpreter>& PythonInterpreter::instance()
{
    const Global<PythonInterpreter> instance;
    return static_cast<const sp<PythonInterpreter>&>(instance);
}

PythonInterpreter::PythonInterpreter()
    : _reference_manager(sp<ReferenceManager>::make())
{
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
    DASSERT(pyObject);
    PyArkType* type = getPyArkType(pyObject);
    DCHECK(type, "Object \"%s\" is not a PyArkType instance", Py_TYPE(pyObject)->tp_name);
    return type;
}

PyObject* PythonInterpreter::toPyObject(const Box& box)
{
    if(!box)
        Py_RETURN_NONE;

    if(box.typeId() == Type<PyInstance>::id())
    {
        PyObject* object = box.as<PyInstance>()->object();
        Py_XINCREF(object);
        return object;
    }

    auto iter = _type_by_id.find(box.typeId());
    DCHECK(iter != _type_by_id.end(), "Unknow box type");

    return iter->second->create(box);
}

bool PythonInterpreter::isPyObject(TypeId type) const
{
    return (type == Type<PyInstance>::id()) || _type_by_id.find(type) != _type_by_id.end();
}

void PythonInterpreter::logErr() const
{
#ifdef ARK_FLAG_DEBUG
    PyErr_Print();
    PyObject* ferr = PySys_GetObject("stderr");
    PyObject* ret = ferr ? PyObject_CallMethod(ferr, "flush", nullptr) : nullptr;
    Py_XDECREF(ret);
#else
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    const char* pStrErrorMessage = PyUnicode_AsUTF8(pvalue);
    PyErr_Restore(ptype, pvalue, ptraceback);
    LOGE("%s", pStrErrorMessage);
#endif
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
    return object != Py_None ? PyInstance::track(object).pack() : Box();
}

template<> ARK_PLUGIN_PYTHON_API bool PythonInterpreter::toType<bool>(PyObject* object)
{
    if(PyBool_Check(object))
        return object == Py_True;
    if(PyLong_Check(object))
        return PyLong_AsLong(object) != 0;
    const sp<Boolean> b = PythonInterpreter::instance()->asInterface<Boolean>(object);
    DCHECK(b, "Casting %s to bool failed", Py_TYPE(object)->tp_name);
    return b->val();
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
    const sp<Vec2> vec2 = toVec2(object);
    if(vec2)
        return vec2->val();
    DFATAL("V2 object should be either Vec2 or length-2 tuple (eg. (1.0, 1.0))");
    return V2();
}

template<> ARK_PLUGIN_PYTHON_API V3 PythonInterpreter::toType<V3>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y, z;
        if(PyArg_ParseTuple(object, "fff", &x, &y, &z))
            return V3(x, y, z);
    }
    const sp<Vec3> vec3 = asInterface<Vec3>(object);
    if(vec3)
        return vec3->val();
    DFATAL("V3 object should be either Vec3 or length-3 tuple (eg. (1.0, 1.0, 1.0))");
    return V3();
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
    DFATAL("Color object should be either int or length-4 float tuple. (eg. 0xffffffff or (1.0, 1.0, 1.0, 1.0))");
    return Color();
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
