#include "python/extension/py_cast.h"

#include "core/base/json.h"
#include "core/base/observer.h"
#include "core/base/scope.h"
#include "core/base/slice.h"
#include "core/inf/variable.h"
#include "core/types/box.h"
#include "core/types/null.h"
#include "core/util/log.h"

#include "graphics/base/color.h"
#include "graphics/base/mat.h"
#include "graphics/base/size.h"
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

#include "python/impl/duck/py_callable_duck_type.h"
#include "python/impl/duck/py_numeric_duck_type.h"
#include "python/impl/duck/py_object_duck_type.h"
#include "python/impl/duck/py_vec_duck_type.h"

namespace ark {
namespace plugin {
namespace python {

Optional<sp<Runnable>> PyCast::toRunnable(PyObject* object, bool alert)
{
    if(PyCallable_Check(object))
        return sp<Runnable>::make<PythonCallableRunnable>(PyInstance::own(object));

    return toSharedPtrDefault<Runnable>(object, alert);
}

Optional<sp<Observer>> PyCast::toObserver(PyObject* object, bool alert)
{
    Optional<sp<Runnable>> runnable = toRunnable(object, false);
    if(runnable)
        return sp<Observer>::make(std::move(runnable.value()));

    return toSharedPtrDefault<Observer>(object, alert);
}

sp<CollisionCallback> PyCast::toCollisionCallback(PyObject* object)
{
    return sp<CollisionCallbackPythonAdapter>::make(PyInstance::borrow(object));
}

sp<EventListener> PyCast::toEventListener(PyObject* object)
{
    if(PyCallable_Check(object))
        return sp<PythonCallableEventListener>::make(PyInstance::track(object));

    if(PyObject_HasAttrString(object, "on_event"))
    {
        PyInstance onEvent = PyInstance::steal(PyObject_GetAttrString(object, "on_event"));
        CHECK(onEvent.isCallable(), "The on_event method of type \"%s\" should be Callable", Py_TYPE(object)->tp_name);
        return sp<PythonCallableEventListener>::make(std::move(onEvent));
    }

    return toSharedPtrDefault<EventListener>(object).value();
}

Optional<String> PyCast::toStringExact(PyObject* object, const char* encoding, const char* error)
{
    if(PyUnicode_Check(object))
        return unicodeToUTF8String(object, encoding, error);
    else if (PyBytes_Check(object))
        return String(PyBytes_AS_STRING(object));
    return Optional<String>();
}

String PyCast::toString(PyObject* object, const char* encoding, const char* error)
{
    if(object)
    {
        Optional<String> opt = toStringExact(object, encoding, error);
        if(opt)
            return opt.value();

        PyObject* str = PyObject_Str(object);
        const String r = unicodeToUTF8String(str, encoding, error);
        Py_DECREF(str);
        return r;
    }
    return "";
}

std::wstring PyCast::toWString(PyObject* object)
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

sp<Vec2> PyCast::toVec2(PyObject* object, bool alert)
{
    if(PyTuple_Check(object) && PyObject_Length(object) == 2)
    {
        PyObject* x, *y;
        if(PyArg_ParseTuple(object, "OO", &x, &y))
            return Vec2Type::create(toNumeric(x).value(), toNumeric(y).value());
        PyErr_Clear();
    }
    sp<Vec2> vec2 = toSharedPtrOrNull<Vec2>(object);

    if(!vec2)
    {
        const sp<Size> size = toSharedPtrOrNull<Size>(object);
        if(size)
            vec2 = Vec2Type::create(size->width(), size->height());
    }

    CHECK(vec2 || !alert, "Cannot cast <%s> to <Vec2>", Py_TYPE(object)->tp_name);
    return vec2;
}

sp<Vec3> PyCast::toVec3(PyObject* object, bool alert)
{
    if(isNoneOrNull(object))
        return nullptr;

    if(PyTuple_Check(object) && (PyObject_Length(object) == 2 || PyObject_Length(object) == 3))
    {
        PyObject* x, *y, *z = nullptr;
        if(PyArg_ParseTuple(object, "OO|O", &x, &y, &z))
            return Vec3Type::create(toNumeric(x).value(), toNumeric(y).value(), toNumeric(z).value());
        PyErr_Clear();
    }
    sp<Vec3> vec3 = toSharedPtrOrNull<Vec3>(object);
    if(vec3)
        return vec3;

    sp<Vec2> vec2 = toSharedPtrOrNull<Vec2>(object);
    if(!vec2 && !alert)
        return nullptr;

    CHECK(vec2, "Cannot cast \"%s\" to Vec3, possible candidates: tuple, Vec3, Vec2", Py_TYPE(object)->tp_name);
    return Vec2Type::extend(vec2, sp<Numeric>::make<Numeric::Const>(0.0f));
}

Optional<sp<Numeric>> PyCast::toNumeric(PyObject* object, bool alert)
{
    if(isNoneOrNull(object))
        return sp<Numeric>::null();

    if(PyLong_Check(object))
        return sp<Numeric>::make<Numeric::Const>(static_cast<float>(PyLong_AsLong(object)));
    if(PyFloat_Check(object))
        return sp<Numeric>::make<Numeric::Const>(static_cast<float>(PyFloat_AsDouble(object)));

    return toSharedPtrDefault<Numeric>(object, alert);
}

Optional<sp<Boolean>> PyCast::toBoolean(PyObject* object, bool alert)
{
    if(isNoneOrNull(object))
        return sp<Boolean>::make<Boolean::Const>(false);

    if(PyBool_Check(object))
        return sp<Boolean>::make<Boolean::Const>(PyObject_IsTrue(object));

    return toSharedPtrDefault<Boolean>(object, alert);
}

Optional<sp<Integer>> PyCast::toInteger(PyObject* object, bool alert)
{
    if(PyLong_CheckExact(object))
        return sp<Integer>::make<Integer::Const>(PyLong_AsLong(object));

    return toSharedPtrDefault<Integer>(object, alert);
}

Scope PyCast::toScope(PyObject* kws)
{
    Scope scope;
    if(kws)
    {
        PythonInterpreter& pi = PythonInterpreter::instance();
        PyObject* keys = PyDict_Keys(kws);
        const Py_ssize_t size = PyList_Size(keys);
        for(Py_ssize_t i = 0; i < size; i ++)
        {
            PyObject* key = PyList_GetItem(keys, i);
            PyObject* item = PyDict_GetItem(kws, key);
            const String sKey = toString(key);
            if(PyTuple_CheckExact(item) || pi.isInstance<Vec2>(item) || pi.isInstance<Vec3>(item) || pi.isInstance<Vec4>(item))
                scope.put(sKey, sp<PyVecDuckType>::make(PyInstance::track(item)));
            else if(pi.isPyArkTypeObject(Py_TYPE(item)))
                scope.put(sKey, *reinterpret_cast<PyArkType::Instance*>(item)->box);
            else if(PyBool_Check(item))
                scope.put(sKey, sp<Boolean::Const>::make(PyObject_IsTrue(item) != 0));
            else if(PyCallable_Check(item))
                scope.put(sKey, sp<PyCallableDuckType>::make(PyInstance::track(item)));
            else
                scope.put(sKey, sp<PyObjectDuckType>::make(PyInstance::track(item)));
        }
        Py_DECREF(keys);
    }
    return scope;
}

PyObject* PyCast::toPyObject_SharedPtr(const sp<PyInstanceRef>& inst)
{
    PyObject* obj = inst->instance();
    Py_XINCREF(obj);
    return obj;
}

PyObject* PyCast::toPyObject_SharedPtr(const sp<String>& inst) {
    if(inst)
        return PyUnicode_FromString(inst->c_str());
    Py_RETURN_NONE;
}

String PyCast::unicodeToUTF8String(PyObject* object, const char* encoding, const char* error)
{
    DCHECK(PyUnicode_Check(object), "Expecting an unicode object");
    return PyUnicode_AsUTF8(object);
}

std::wstring PyCast::pyUnicodeToWString(PyObject* unicode)
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

PyObject* PyCast::toPyObject(const Box& box)
{
    return PythonInterpreter::instance()->toPyObject(box);
}

bool PyCast::isNoneOrNull(PyObject* pyObject)
{
    return !pyObject || pyObject == Py_None;
}

template<> ARK_PLUGIN_PYTHON_API Optional<String> PyCast::toCppObject_impl<String>(PyObject* object)
{
    return toStringExact(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<std::wstring> PyCast::toCppObject_impl<std::wstring>(PyObject* object)
{
    return toWString(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<Json> PyCast::toCppObject_impl<Json>(PyObject* object)
{
    Optional<sp<Json>> opt = toSharedPtr<Json>(object);
    return opt ? Optional<Json>(std::move(*opt.value())) : Optional<Json>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<Box> PyCast::toCppObject_impl<Box>(PyObject* object)
{
    if(PythonInterpreter::instance()->isPyArkTypeObject(Py_TYPE(object)))
        return *reinterpret_cast<PyArkType::Instance*>(object)->box;
    return object != Py_None ? Box(PyInstance::track(object).ref()) : Box();
}

template<> ARK_PLUGIN_PYTHON_API Optional<bool> PyCast::toCppObject_impl<bool>(PyObject* object)
{
    if(object == Py_None)
        return false;
    if(PyBool_Check(object))
        return object == Py_True;
    if(PyLong_Check(object))
        return PyLong_AsLong(object) != 0;
    const sp<Boolean> b = toSharedPtrDefault<Boolean>(object).value();
    DCHECK(b, "Casting %s to bool failed", Py_TYPE(object)->tp_name);
    return b->val();
}

template<> ARK_PLUGIN_PYTHON_API Optional<float> PyCast::toCppObject_impl<float>(PyObject* object)
{
    DCHECK(PyNumber_Check(object), "Cannot cast Python object \"%s\" to float", object->ob_type->tp_name);
    return static_cast<float>(PyFloat_AsDouble(object));
}

template<> ARK_PLUGIN_PYTHON_API Optional<uint32_t> PyCast::toCppObject_impl<uint32_t>(PyObject* object)
{
    PythonInterpreter& pi = PythonInterpreter::instance();
    if(pi.isPyArkTypeObject(object))
    {
        PyArkType* pyArkType = pi.getPyArkType(object);
        DCHECK(pyArkType, "Cannot convert PyObject to PyArkType");
        return pyArkType->typeId();
    }
    if(PyLong_Check(object))
        return PyLong_AsUnsignedLong(object);
    return Optional<uint32_t>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<int32_t> PyCast::toCppObject_impl<int32_t>(PyObject* object)
{
    if(PyLong_Check(object))
        return static_cast<int32_t>(PyLong_AsLong(object));
    return Optional<int32_t>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<uint16_t> PyCast::toCppObject_impl<uint16_t>(PyObject* object)
{
    if(PyLong_CheckExact(object))
        return static_cast<uint16_t>(PyLong_AsUnsignedLong(object));
    return Optional<uint16_t>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<int64_t> PyCast::toCppObject_impl<int64_t>(PyObject* object)
{
    if(PyLong_Check(object))
        return static_cast<int64_t>(PyLong_AsLongLong(object));
    return Optional<int64_t>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<uint64_t> PyCast::toCppObject_impl<uint64_t>(PyObject* object)
{
    if(PyLong_Check(object))
        return static_cast<uint64_t>(PyLong_AsUnsignedLongLong(object));
    return Optional<uint64_t>();
}

#ifdef __APPLE__
template<> ARK_PLUGIN_PYTHON_API Optional<size_t> PyCast::toCppObject_impl<size_t>(PyObject* object)
{
    CHECK(PyNumber_Check(object), "Cannot cast Python object \"%s\" to size_t", object->ob_type->tp_name);
    return static_cast<size_t>(PyLong_AsUnsignedLongLong(object));
}
#endif

template<> ARK_PLUGIN_PYTHON_API Optional<V2> PyCast::toCppObject_impl<V2>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y;
        if(PyArg_ParseTuple(object, "ff", &x, &y))
            return V2(x, y);
        PyErr_Clear();
    }
    const sp<Vec2> vec2 = toVec2(object, true);
    if(vec2)
        return vec2->val();
    FATAL("V2 object should be either Vec2 or length-2 tuple (eg. (1.0, 1.0))");
    return V2();
}

template<> ARK_PLUGIN_PYTHON_API Optional<V3> PyCast::toCppObject_impl<V3>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y, z = 0;
        if(PyArg_ParseTuple(object, "ff|f", &x, &y, &z))
            return V3(x, y, z);
        PyErr_Clear();
    }
    Optional<sp<Vec3>> vec3 = toSharedPtr<Vec3>(object);
    if(vec3)
        return vec3.value()->val();
    return Optional<V3>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<V4> PyCast::toCppObject_impl<V4>(PyObject* object)
{
    if(PyTuple_Check(object))
    {
        float x, y, z = 0, w = 0;
        if(PyArg_ParseTuple(object, "ff|ff", &x, &y, &z, &w))
            return V4(x, y, z, w);
        PyErr_Clear();
    }
    Optional<sp<Vec4>> vec4 = toSharedPtr<Vec4>(object);
    if(vec4)
        return vec4.value()->val();
    return Optional<V4>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<M4> PyCast::toCppObject_impl<M4>(PyObject* object)
{
    if(PyTuple_Check(object) && PyTuple_Size(object) == 4)
    {
        M4 ret;
        for(Py_ssize_t i = 0; i < 4; ++i)
        {
            float* fp = reinterpret_cast<float*>(&ret) + i * 4;
            if(!PyArg_ParseTuple(PyTuple_GetItem(object, i), "ffff", fp, fp + 1, fp + 2, fp + 3))
            {
                PyErr_Clear();
                break;
            }
        }
        return ret;
    }
    return Optional<M4>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<Slice> PyCast::toCppObject_impl<Slice>(PyObject* object)
{
    if(PySlice_Check(object))
    {
        Py_ssize_t begin = 0, end = -1, step = 1;
        int32_t r = PySlice_Unpack(object, &begin, &end, &step);
        CHECK(r == 0, "PySlice_Unpack failed: %d", r);
        return Slice(begin, end, step);
    }
    return Optional<Slice>();
}

template<typename T> RectT<T> toRectType(PyObject* obj)
{
    DCHECK(PyTuple_Check(obj) && PyTuple_Size(obj) == 4, "Rect object should be 4-length tuple");
    const T arg0 = PyCast::toCppObject_impl<T>(PyTuple_GetItem(obj, 0)).value();
    const T arg1 = PyCast::toCppObject_impl<T>(PyTuple_GetItem(obj, 1)).value();
    const T arg2 = PyCast::toCppObject_impl<T>(PyTuple_GetItem(obj, 2)).value();
    const T arg3 = PyCast::toCppObject_impl<T>(PyTuple_GetItem(obj, 3)).value();
    return RectT<T>(arg0, arg1, arg2, arg3);
}

template<> ARK_PLUGIN_PYTHON_API Optional<RectF> PyCast::toCppObject_impl<RectF>(PyObject* object)
{
    return toRectType<float>(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<RectI> PyCast::toCppObject_impl<RectI>(PyObject* object)
{
    return toRectType<int32_t>(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<Color> PyCast::toCppObject_impl<Color>(PyObject* object)
{
    if(PyLong_Check(object))
        return Color(static_cast<uint32_t>(PyLong_AsLongLong(object)));
    if(PyTuple_Check(object))
    {
        float r, g, b, a = 1.0f;
        if(PyArg_ParseTuple(object, "fff|f", &r, &g, &b, &a))
            return Color(r, g, b, a);
        PyErr_Clear();
    }
    FATAL("Color object should be either int or length-4 float tuple. (eg. 0xffffffff or (1.0, 1.0, 1.0, 1.0))");
    return Color();
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<Box>(const Box& value)
{
    return toPyObject(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<String>(const String& value)
{
    return PyUnicode_FromString(value.c_str());
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<std::wstring>(const std::wstring& value)
{
    return PyUnicode_FromKindAndData(sizeof(wchar_t), value.c_str(), value.length());
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<Json>(const Json& value)
{
    return toPyObject_SharedPtr<Json>(sp<Json>::make(value));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<Buffer>(const Buffer& buffer)
{
    return toPyObject_SharedPtr<Buffer>(sp<Buffer>::make(buffer));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<bool>(const bool& value)
{
    return PyBool_FromLong(value);
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<V2>(const V2& value)
{
    PyObject* v2 = PyTuple_New(2);
    PyTuple_SetItem(v2, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v2, 1, PyFloat_FromDouble(value.y()));
    return v2;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<V3>(const V3& value)
{
    PyObject* v3 = PyTuple_New(3);
    PyTuple_SetItem(v3, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v3, 1, PyFloat_FromDouble(value.y()));
    PyTuple_SetItem(v3, 2, PyFloat_FromDouble(value.z()));
    return v3;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<V4>(const V4& value)
{
    PyObject* v4 = PyTuple_New(4);
    PyTuple_SetItem(v4, 0, PyFloat_FromDouble(value.x()));
    PyTuple_SetItem(v4, 1, PyFloat_FromDouble(value.y()));
    PyTuple_SetItem(v4, 2, PyFloat_FromDouble(value.z()));
    PyTuple_SetItem(v4, 3, PyFloat_FromDouble(value.w()));
    return v4;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<M4>(const M4& value)
{
    PyObject* m4 = PyTuple_New(4);
    const V4* ptr = reinterpret_cast<const V4*>(&value);
    for(Py_ssize_t i = 0; i < 4; ++i)
        PyTuple_SetItem(m4, i, toPyObject_impl<V4>(*(ptr ++)));
    return m4;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<Rect>(const Rect& value)
{
    PyObject* v4 = PyTuple_New(4);
    PyTuple_SetItem(v4, 0, PyFloat_FromDouble(value.left()));
    PyTuple_SetItem(v4, 1, PyFloat_FromDouble(value.top()));
    PyTuple_SetItem(v4, 2, PyFloat_FromDouble(value.right()));
    PyTuple_SetItem(v4, 3, PyFloat_FromDouble(value.bottom()));
    return v4;
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<Color>(const Color& color)
{
    return toPyObject_SharedPtr<Color>(sp<Color>::make(color));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<RayCastManifold>(const RayCastManifold& manifold)
{
    return toPyObject_SharedPtr<RayCastManifold>(sp<RayCastManifold>::make(manifold));
}

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<Span>(const Span& strview)
{
    return PyBytes_FromStringAndSize(strview.data(), static_cast<size_t>(strview.size()));
}

}
}
}
