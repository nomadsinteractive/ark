#include "python/extension/python_interpreter.h"

#include "core/base/json.h"
#include "core/base/observer.h"
#include "core/base/scope.h"
#include "core/inf/variable.h"
#include "core/types/box.h"
#include "core/types/global.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "graphics/base/color.h"
#include "graphics/util/vec2_type.h"
#include "graphics/util/vec3_type.h"

#include "app/base/event.h"
#include "app/base/raycast_manifold.h"

#include "python/api.h"
#include "python/extension/python_interpreter.h"
#include "python/extension/py_instance_ref.h"
#include "python/extension/reference_manager.h"

#include "python/impl/adapter/collision_callback_python_adapter.h"
#include "python/impl/adapter/python_callable_runnable.h"
#include "python/impl/adapter/python_callable_event_listener.h"

#include "python/impl/duck/py_array_duck_type.h"
#include "python/impl/duck/py_callable_duck_type.h"
#include "python/impl/duck/py_numeric_duck_type.h"
#include "python/impl/duck/py_object_duck_type.h"
#include "python/impl/duck/py_vec_duck_type.h"

namespace ark {
namespace plugin {
namespace python {

sp<Runnable> PythonInterpreter::toRunnable(PyObject* object, bool alert)
{
    if(PyCallable_Check(object))
        return sp<PythonCallableRunnable>::make(PyInstance::own(object));

    return toSharedPtrImpl<Runnable>(object, alert);
}

sp<Observer> PythonInterpreter::toObserver(PyObject* object, bool alert)
{
    sp<Runnable> runnable = toRunnable(object, false);
    if(runnable)
        return sp<Observer>::make(runnable);

    return toSharedPtrImpl<Observer>(object, alert);
}

sp<CollisionCallback> PythonInterpreter::toCollisionCallback(PyObject* object)
{
    return sp<CollisionCallbackPythonAdapter>::make(PyInstance::borrow(object));
}

sp<EventListener> PythonInterpreter::toEventListener(PyObject* object)
{
    if(PyCallable_Check(object))
        return sp<PythonCallableEventListener>::make(PyInstance::track(object));

    return toSharedPtrImpl<EventListener>(object);
}

String PythonInterpreter::toString(PyObject* object, const char* encoding, const char* error) const
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

std::wstring PythonInterpreter::toWString(PyObject* object) const
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

sp<Vec2> PythonInterpreter::toVec2(PyObject* object, bool alert)
{
    if(PyTuple_Check(object) && PyObject_Length(object) == 2)
    {
        PyObject* x, *y;
        if(PyArg_ParseTuple(object, "OO", &x, &y))
            return Vec2Type::create(toNumeric(x), toNumeric(y));
    }
    return toSharedPtrImpl<Vec2>(object, alert);
}

sp<Vec3> PythonInterpreter::toVec3(PyObject* object, bool alert)
{
    if(isNoneOrNull(object))
        return nullptr;

    if(PyTuple_Check(object) && (PyObject_Length(object) == 2 || PyObject_Length(object) == 3))
    {
        PyObject* x, *y, *z = nullptr;
        if(PyArg_ParseTuple(object, "OO|O", &x, &y, &z))
            return Vec3Type::create(toNumeric(x), toNumeric(y), toNumeric(z));
    }
    const sp<Vec3> vec3 = toSharedPtrImpl<Vec3>(object, false);
    if(vec3)
        return vec3;

    const sp<Vec2> vec2 = toSharedPtrImpl<Vec2>(object, false);
    if(!vec2 && !alert)
        return nullptr;

    DCHECK(vec2, "Cannot cast \"%s\" to Vec3, possible candidates: tuple, Vec3, Vec2", Py_TYPE(object)->tp_name);
    return Vec3Type::create(vec2);
}

PyObject* PythonInterpreter::toPyObject_SharedPtr(const bytearray& bytes)
{
    return PyBytes_FromStringAndSize(reinterpret_cast<const char*>(bytes->buf()), static_cast<size_t>(bytes->length()));
}

sp<Numeric> PythonInterpreter::toNumeric(PyObject* object, bool alert)
{
    if(isNoneOrNull(object))
        return nullptr;

    if(PyLong_Check(object))
        return sp<Numeric::Const>::make(static_cast<float>(PyLong_AsLong(object)));
    if(PyFloat_Check(object))
        return sp<Numeric::Const>::make(static_cast<float>(PyFloat_AsDouble(object)));

    return toSharedPtrImpl<Numeric>(object, alert);
}

sp<Boolean> PythonInterpreter::toBoolean(PyObject* object, bool alert)
{
    if(isNoneOrNull(object))
        return sp<Boolean>::make<Boolean::Const>(false);

    if(PyBool_Check(object))
        return sp<Boolean>::make<Boolean::Const>(PyObject_IsTrue(object));

    return toSharedPtrImpl<Boolean>(object, alert);
}

sp<Integer> PythonInterpreter::toInteger(PyObject* object, bool alert)
{
    if(PyLong_CheckExact(object))
        return sp<Integer::Const>::make(PyLong_AsLong(object));

    return toSharedPtrImpl<Integer>(object, alert);
}

Scope PythonInterpreter::toScope(PyObject* kws) const
{
    Scope scope;
    if(kws)
    {
        PyObject* keys = PyDict_Keys(kws);
        const Py_ssize_t size = PyList_Size(keys);
        for(Py_ssize_t i = 0; i < size; i ++)
        {
            PyObject* key = PyList_GetItem(keys, i);
            PyObject* item = PyDict_GetItem(kws, key);
            const String sKey = toString(key);
            if(PyTuple_CheckExact(item) || isInstance<Vec2>(item) || isInstance<Vec3>(item) || isInstance<Vec4>(item))
                scope.put(sKey, sp<PyVecDuckType>::make(PyInstance::track(item)));
            else if(isPyArkTypeObject(Py_TYPE(item)))
                scope.put(sKey, *reinterpret_cast<PyArkType::Instance*>(item)->box);
            else if(PyBool_Check(item))
                scope.put(sKey, sp<Boolean::Const>::make(PyObject_IsTrue(item) != 0));
            else if(PyCallable_Check(item))
                scope.put(sKey, sp<PyCallableDuckType>::make(PyInstance::track(item)));
            else if(PyList_CheckExact(item))
                scope.put(sKey, sp<PyArrayDuckType>::make(PyInstance::track(item)));
            else
                scope.put(sKey, sp<PyObjectDuckType>::make(PyInstance::track(item)));
        }
        Py_DECREF(keys);
    }
    return scope;
}

PyObject* PythonInterpreter::toPyObject_SharedPtr(const sp<PyInstanceRef>& inst)
{
    PyObject* obj = inst->instance();
    Py_XINCREF(obj);
    return obj;
}

PyObject* PythonInterpreter::toPyObject_SharedPtr(const sp<String>& inst) {
    if(inst)
        return PyUnicode_FromString(inst->c_str());
    Py_RETURN_NONE;
}

String PythonInterpreter::unicodeToUTF8String(PyObject* object, const char* encoding, const char* error) const
{
    DCHECK(PyUnicode_Check(object), "Expecting an unicode object");
    return PyUnicode_AsUTF8(object);
}

std::wstring PythonInterpreter::pyUnicodeToWString(PyObject* unicode) const
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
    DCHECK(iter != _type_by_id.end(), "Unknow box type: %s", Class::getClass(box.typeId())->name());
    return iter->second->create(box);
}

bool PythonInterpreter::isPyObject(TypeId type) const
{
    return (type == Type<PyInstance>::id()) || _type_by_id.find(type) != _type_by_id.end();
}

bool PythonInterpreter::isNoneOrNull(PyObject* pyObject) const
{
    return !pyObject || pyObject == Py_None;
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

template<> ARK_PLUGIN_PYTHON_API String PythonInterpreter::toCppObject_impl<String>(PyObject* object)
{
    return toString(object);
}

template<> ARK_PLUGIN_PYTHON_API std::wstring PythonInterpreter::toCppObject_impl<std::wstring>(PyObject* object)
{
    return toWString(object);
}

template<> ARK_PLUGIN_PYTHON_API Json PythonInterpreter::toCppObject_impl<Json>(PyObject* object)
{
    return *toSharedPtr<Json>(object);
}

template<> ARK_PLUGIN_PYTHON_API Box PythonInterpreter::toCppObject_impl<Box>(PyObject* object)
{
    return object != Py_None ? Box(PyInstance::track(object).ref()) : Box();
}

template<> ARK_PLUGIN_PYTHON_API bool PythonInterpreter::toCppObject_impl<bool>(PyObject* object)
{
    if(object == Py_None)
        return false;
    if(PyBool_Check(object))
        return object == Py_True;
    if(PyLong_Check(object))
        return PyLong_AsLong(object) != 0;
    const sp<Boolean> b = PythonInterpreter::instance()->toSharedPtrImpl<Boolean>(object);
    DCHECK(b, "Casting %s to bool failed", Py_TYPE(object)->tp_name);
    return b->val();
}

template<> ARK_PLUGIN_PYTHON_API float PythonInterpreter::toCppObject_impl<float>(PyObject* object)
{
    DCHECK(PyNumber_Check(object), "Cannot cast Python object \"%s\" to float", object->ob_type->tp_name);
    return static_cast<float>(PyFloat_AsDouble(object));
}

template<> ARK_PLUGIN_PYTHON_API uint32_t PythonInterpreter::toCppObject_impl<uint32_t>(PyObject* object)
{
    if(isPyArkTypeObject(object))
    {
        PyArkType* pyArkType = PythonInterpreter::instance()->getPyArkType(object);
        DCHECK(pyArkType, "Cannot convert PyObject to PyArkType");
        return pyArkType->typeId();
    }
    return PyLong_AsUnsignedLong(object);
}

template<> ARK_PLUGIN_PYTHON_API int32_t PythonInterpreter::toCppObject_impl<int32_t>(PyObject* object)
{
    DCHECK(PyNumber_Check(object), "Cannot cast Python object \"%s\" to int32_t", object->ob_type->tp_name);
    return static_cast<int32_t>(PyLong_AsLong(object));
}

template<> ARK_PLUGIN_PYTHON_API V2 PythonInterpreter::toCppObject_impl<V2>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y;
        if(PyArg_ParseTuple(object, "ff", &x, &y))
            return V2(x, y);
    }
    const sp<Vec2> vec2 = toVec2(object, true);
    if(vec2)
        return vec2->val();
    DFATAL("V2 object should be either Vec2 or length-2 tuple (eg. (1.0, 1.0))");
    return V2();
}

template<> ARK_PLUGIN_PYTHON_API V3 PythonInterpreter::toCppObject_impl<V3>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y, z = 0;
        if(PyArg_ParseTuple(object, "ff|f", &x, &y, &z))
            return V3(x, y, z);
    }
    const sp<Vec3> vec3 = toSharedPtr<Vec3>(object);
    if(vec3)
        return vec3->val();
    DFATAL("V3 object should be either Vec3 or length-3 tuple (eg. (1.0, 1.0, 1.0))");
    return V3();
}

template<> ARK_PLUGIN_PYTHON_API V4 PythonInterpreter::toCppObject_impl<V4>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y, z = 0, w = 0;
        if(PyArg_ParseTuple(object, "ff|ff", &x, &y, &z, &w))
            return V4(x, y, z, w);
    }
    const sp<Vec4> vec4 = toSharedPtr<Vec4>(object);
    if(vec4)
        return vec4->val();
    DFATAL("V4 object should be either Vec4 or length-4 tuple (eg. (1.0, 1.0, 1.0, 1.0))");
    return V4();
}

template<typename T> RectT<T> toRectType(PyObject* obj)
{
    DCHECK(PyTuple_Check(obj) && PyTuple_Size(obj) == 4, "Rect object should be 4-length tuple");
    PythonInterpreter& interpreter = PythonInterpreter::instance();
    const T arg0 = interpreter.toCppObject_impl<T>(PyTuple_GetItem(obj, 0));
    const T arg1 = interpreter.toCppObject_impl<T>(PyTuple_GetItem(obj, 1));
    const T arg2 = interpreter.toCppObject_impl<T>(PyTuple_GetItem(obj, 2));
    const T arg3 = interpreter.toCppObject_impl<T>(PyTuple_GetItem(obj, 3));
    return RectT<T>(arg0, arg1, arg2, arg3);
}

template<> ARK_PLUGIN_PYTHON_API RectF PythonInterpreter::toCppObject_impl<RectF>(PyObject* object)
{
    return toRectType<float>(object);
}

template<> ARK_PLUGIN_PYTHON_API RectI PythonInterpreter::toCppObject_impl<RectI>(PyObject* object)
{
    return toRectType<int32_t>(object);
}

template<> ARK_PLUGIN_PYTHON_API Color PythonInterpreter::toCppObject_impl<Color>(PyObject* object)
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

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<Box>(const Box& value)
{
    return toPyObject(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<String>(const String& value)
{
    return PyUnicode_FromString(value.c_str());
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<std::wstring>(const std::wstring& value)
{
    return PyUnicode_FromKindAndData(sizeof(wchar_t), value.c_str(), value.length());
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<Json>(const Json& value)
{
    return toPyObject_SharedPtr<Json>(sp<Json>::make(value));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<bool>(const bool& value)
{
    return PyBool_FromLong(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<V2>(const V2& value)
{
    PyObject* v2 = PyTuple_New(2);
    PyTuple_SetItem(v2, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v2, 1, PyFloat_FromDouble(value.y()));
    return v2;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<V3>(const V3& value)
{
    PyObject* v3 = PyTuple_New(3);
    PyTuple_SetItem(v3, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v3, 1, PyFloat_FromDouble(value.y()));
    PyTuple_SetItem(v3, 2, PyFloat_FromDouble(value.z()));
    return v3;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<V4>(const V4& value)
{
    PyObject* v4 = PyTuple_New(4);
    PyTuple_SetItem(v4, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v4, 1, PyFloat_FromDouble(value.y()));
    PyTuple_SetItem(v4, 2, PyFloat_FromDouble(value.z()));
    PyTuple_SetItem(v4, 3, PyFloat_FromDouble(value.w()));
    return v4;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<Rect>(const Rect& value)
{
    PyObject* v4 = PyTuple_New(4);
    PyTuple_SetItem(v4, 0, PyFloat_FromDouble(value.left()));
    PyTuple_SetItem(v4, 1, PyFloat_FromDouble(value.top()));
    PyTuple_SetItem(v4, 2, PyFloat_FromDouble(value.right()));
    PyTuple_SetItem(v4, 3, PyFloat_FromDouble(value.bottom()));
    return v4;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<Color>(const Color& color)
{
    return PythonInterpreter::instance()->toPyObject_SharedPtr<Color>(sp<Color>::make(color));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PythonInterpreter::toPyObject_impl<RayCastManifold>(const RayCastManifold& manifold)
{
    return PythonInterpreter::instance()->toPyObject_SharedPtr<RayCastManifold>(sp<RayCastManifold>::make(manifold));
}

}
}
}
