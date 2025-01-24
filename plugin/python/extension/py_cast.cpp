#include "python/extension/py_cast.h"

#include "core/base/json.h"
#include "core/base/observer.h"
#include "core/base/named_hash.h"
#include "core/base/scope.h"
#include "core/base/slice.h"
#include "core/inf/variable.h"
#include "core/types/box.h"
#include "core/util/log.h"

#include "graphics/base/color.h"
#include "graphics/base/mat.h"
#include "graphics/util/vec2_type.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/vec4_type.h"

#include "app/base/event.h"
#include "app/base/raycast_manifold.h"

#include "python/api.h"
#include "python/extension/python_extension.h"
#include "python/extension/py_instance_ref.h"

#include "python/impl/adapter/collision_callback_python.h"
#include "python/impl/adapter/runnable_python.h"
#include "python/impl/adapter/event_listener_python.h"

#include "python/impl/duck/py_callable_duck_type.h"
#include "python/impl/duck/py_list_duck_type.h"
#include "python/impl/duck/py_object_duck_type.h"

namespace ark::plugin::python {

namespace {

template<typename T> Optional<T> toCppInteger(PyObject* object)
{
    if(PyLong_Check(object))
    {
        if constexpr(sizeof(T) == sizeof(uint64_t))
        {
            if constexpr(std::is_unsigned_v<T>)
                return {PyLong_AsUnsignedLongLong(object)};
            else
                return {PyLong_AsLongLong(object)};
        }
        if constexpr(std::is_unsigned_v<T>)
            return {static_cast<T>(PyLong_AsUnsignedLong(object))};
        else
            return {static_cast<T>(PyLong_AsLong(object))};
    }

    if(PyIndex_Check(object))
    {
        PyObject* pyobj = PyNumber_Index(object);
        Optional<T> opt = PyLong_AsUnsignedLong(pyobj);
        Py_DECREF(pyobj);
        return opt;
    }
    return {};
}

std::wstring pyUnicodeToWString(PyObject* unicode)
{
    PyUnicode_READY(unicode);
    uint32_t k = PyUnicode_KIND(unicode);
    void* data = PyUnicode_DATA(unicode);
    if(k == sizeof(wchar_t))
        return static_cast<wchar_t*>(data);
    std::vector<wchar_t> buf;
    if(k == 1)
        for(char* i = static_cast<char*>(data); *i; ++i)
            buf.push_back(static_cast<wchar_t>(*i));
    if(k == 2)
        for(uint16_t* i = static_cast<uint16_t*>(data); *i; ++i)
            buf.push_back(static_cast<wchar_t>(*i));
    if(k == 4)
        for(uint32_t* i = static_cast<uint32_t*>(data); *i; ++i)
            buf.push_back(static_cast<wchar_t>(*i));
    return std::wstring(buf.begin(), buf.end());
}

std::wstring toWString(PyObject* object)
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

String unicodeToUTF8String(PyObject* object, const char* encoding, const char* error)
{
    DCHECK(PyUnicode_Check(object), "Expecting an unicode object");
    return PyUnicode_AsUTF8(object);
}

}

Optional<sp<Runnable>> PyCast::toRunnable(PyObject* object)
{
    if(PyCallable_Check(object))
        return sp<Runnable>::make<RunnablePython>(PyInstance::own(object));

    return toSharedPtrDefault<Runnable>(object);
}

Optional<String> PyCast::toStringExact(PyObject* object, const char* encoding, const char* error)
{
    if(PyUnicode_Check(object))
        return unicodeToUTF8String(object, encoding, error);
    if (PyBytes_Check(object))
        return String(PyBytes_AS_STRING(object));
    return {};
}

String PyCast::toString(PyObject* object, const char* encoding, const char* error)
{
    if(object)
    {
        if(Optional<String> opt = toStringExact(object, encoding, error))
            return opt.value();

        PyObject* str = PyObject_Str(object);
        const String r = unicodeToUTF8String(str, encoding, error);
        Py_DECREF(str);
        return r;
    }
    return {};
}

sp<Vec2> PyCast::toVec2(PyObject* object)
{
    if(PyTuple_CheckExact(object) && PyObject_Length(object) == 2)
    {
        PyObject* x, *y;
        if(PyArg_ParseTuple(object, "OO", &x, &y))
            return Vec2Type::create(toNumeric(x).value(), toNumeric(y).value());
        PyErr_Clear();
    }
    return toSharedPtrOrNull<Vec2>(object);
}

sp<Vec3> PyCast::toVec3(PyObject* object)
{
    if(PyTuple_CheckExact(object) && PyObject_Length(object) == 3)
    {
        PyObject* x, *y, *z;
        if(PyArg_ParseTuple(object, "OOO", &x, &y, &z))
            return Vec3Type::create(toNumeric(x).value(), toNumeric(y).value(), toNumeric(z).value());
        PyErr_Clear();
    }
    return toSharedPtrOrNull<Vec3>(object);
}

sp<Vec4> PyCast::toVec4(PyObject* object)
{
    if(PyTuple_CheckExact(object) && PyObject_Length(object) == 4)
    {
        PyObject* x, *y, *z, *w = nullptr;
        if(PyArg_ParseTuple(object, "OOOO", &x, &y, &z, &w))
            return Vec4Type::create(toNumeric(x).value(), toNumeric(y).value(), toNumeric(z).value(), toNumeric(w).value());
        PyErr_Clear();
    }
    return toSharedPtrOrNull<Vec4>(object);
}

Optional<sp<Mat4>> PyCast::toMat4(PyObject* object)
{
    if(const Optional<M4> matrix = toCppObject<M4>(object))
        return {sp<Mat4>::make<Mat4::Const>(matrix.value())};
    return toSharedPtrDefault<Mat4>(object);
}

Optional<sp<StringVar>> PyCast::toStringVar(PyObject* object)
{
    if(Optional<String> opt = toStringExact(object))
        return sp<StringVar>::make<StringVar::Const>(sp<String>::make(opt.value()));

    return toSharedPtrDefault<StringVar>(object);
}

Optional<sp<Numeric>> PyCast::toNumeric(PyObject* object)
{
    if(isNoneOrNull(object))
        return {sp<Numeric>()};

    if(PyLong_Check(object))
        return {sp<Numeric>::make<Numeric::Const>(static_cast<float>(PyLong_AsLong(object)))};
    if(PyFloat_Check(object))
        return {sp<Numeric>::make<Numeric::Const>(static_cast<float>(PyFloat_AsDouble(object)))};

    return toSharedPtrDefault<Numeric>(object);
}

Optional<sp<Boolean>> PyCast::toBoolean(PyObject* object)
{
    if(isNoneOrNull(object))
        return sp<Boolean>::make<Boolean::Const>(false);

    if(PyBool_Check(object))
        return sp<Boolean>::make<Boolean::Const>(PyObject_IsTrue(object));

    return toSharedPtrDefault<Boolean>(object);
}

Optional<sp<Integer>> PyCast::toInteger(PyObject* object)
{
    if(PyLong_CheckExact(object))
        return sp<Integer>::make<Integer::Const>(PyLong_AsLong(object));

    return toSharedPtrDefault<Integer>(object);
}

Scope PyCast::toScope(PyObject* kws)
{
    Scope scope;
    if(kws)
    {
        const PythonExtension& pi = PythonExtension::instance();
        PyObject* keys = PyDict_Keys(kws);
        const Py_ssize_t size = PyList_Size(keys);
        for(Py_ssize_t i = 0; i < size; i ++)
        {
            PyObject* key = PyList_GetItem(keys, i);
            PyObject* item = PyDict_GetItem(kws, key);
            const String sKey = toString(key);
            if(PyList_CheckExact(item) || PyTuple_CheckExact(item))
                scope.put(sKey, Box(sp<PyListDuckType>::make(PyInstance::track(item))));
            else if(pi.isPyArkTypeObject(Py_TYPE(item)))
                scope.put(sKey, *reinterpret_cast<PyArkType::Instance*>(item)->box);
            else if(PyBool_Check(item))
                scope.put(sKey, Box(sp<Boolean>::make<Boolean::Const>(PyObject_IsTrue(item) != 0)));
            else if(PyCallable_Check(item))
                scope.put(sKey, Box(sp<PyCallableDuckType>::make(PyInstance::track(item))));
            else if(Py_IsNone(item))
                scope.put(sKey, {});
            else
                scope.put(sKey, Box(sp<PyObjectDuckType>::make(PyInstance::track(item))));
        }
        Py_DECREF(keys);
    }
    return scope;
}

Traits PyCast::toTraits(PyObject* args, size_t offset)
{
    Traits traits;
    if(const Py_ssize_t size = PyTuple_Size(args); size > 0)
        for(size_t i = offset; i < size; ++i)
        {
            PyObject* v = PyTuple_GetItem(args, i);
            DCHECK(PythonExtension::instance().isPyArkTypeObject(Py_TYPE(v)), "Trait \"%s\" must be an Ark Type", Py_TYPE(v)->tp_name);
            Box component(*reinterpret_cast<PyArkType::Instance*>(v)->box);
            const TypeId typeId = component.typeId();
            traits.put(typeId, std::move(component));
        }
    return traits;
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

PyObject* PyCast::toPyObject(const Box& box)
{
    return PythonExtension::instance().toPyObject(box);
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

template<> ARK_PLUGIN_PYTHON_API Optional<NamedHash> PyCast::toCppObject_impl<NamedHash>(PyObject* object)
{
    if(Optional<String> strOpt = toStringExact(object))
        return NamedHash(std::move(strOpt.value()));
    if(Optional<int32_t> intOpt = toCppInteger<int32_t>(object))
        return NamedHash(intOpt.value());
    return {};
}

template<> ARK_PLUGIN_PYTHON_API Optional<Json> PyCast::toCppObject_impl<Json>(PyObject* object)
{
    Optional<sp<Json>> opt = toSharedPtr<Json>(object);
    return opt ? Optional<Json>(std::move(*opt.value())) : Optional<Json>();
}

template<> ARK_PLUGIN_PYTHON_API Optional<Box> PyCast::toCppObject_impl<Box>(PyObject* object)
{
    if(PythonExtension::instance().isPyArkTypeObject(Py_TYPE(object)))
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
    PythonExtension& pi = PythonExtension::instance();
    if(pi.isPyArkTypeObject(object))
    {
        PyArkType* pyArkType = pi.getPyArkType(object);
        DCHECK(pyArkType, "Cannot convert PyObject to PyArkType");
        return pyArkType->typeId();
    }
    return toCppInteger<uint32_t>(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<int32_t> PyCast::toCppObject_impl<int32_t>(PyObject* object)
{
    return toCppInteger<int32_t>(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<uint16_t> PyCast::toCppObject_impl<uint16_t>(PyObject* object)
{
    return toCppInteger<uint16_t>(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<uint8_t> PyCast::toCppObject_impl<uint8_t>(PyObject* object)
{
    return toCppInteger<uint8_t>(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<int64_t> PyCast::toCppObject_impl<int64_t>(PyObject* object)
{
    return toCppInteger<int64_t>(object);
}

template<> ARK_PLUGIN_PYTHON_API Optional<uint64_t> PyCast::toCppObject_impl<uint64_t>(PyObject* object)
{
    return toCppInteger<uint64_t>(object);
}

#ifdef __APPLE__
template<> ARK_PLUGIN_PYTHON_API Optional<size_t> PyCast::toCppObject_impl<size_t>(PyObject* object)
{
    return toCppInteger<size_t>(object);
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
    if(const Optional<sp<Vec2>> vec2 = toSharedPtr<Vec2>(object))
        return vec2.value()->val();
    FATAL("V2 object should be either Vec2 or length-2 tuple (eg. (1.0, 1.0))");
    return {};
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
    if(const Optional<sp<Vec3>> vec3 = toSharedPtr<Vec3>(object))
        return vec3.value()->val();
    return {};
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

    if(Optional<sp<Vec4>> vec4 = toSharedPtr<Vec4>(object))
        return vec4.value()->val();
    return {};
}

template<> ARK_PLUGIN_PYTHON_API Optional<M4> PyCast::toCppObject_impl<M4>(PyObject* object)
{
    if(PyTuple_Check(object) && PyTuple_Size(object) == 4)
    {
        M4 ret;
        for(Py_ssize_t i = 0; i < 4; ++i)
        {
            PyObject* row = PyTuple_GetItem(object, i);
            if(PyTuple_CheckExact(row))
            {
                float* fp = reinterpret_cast<float*>(&ret) + i * 4;
                if(!PyArg_ParseTuple(row, "ffff", fp, fp + 1, fp + 2, fp + 3))
                {
                    PyErr_Clear();
                    break;
                }
            }
            else
                return Optional<M4>();
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

template<> ARK_PLUGIN_PYTHON_API PyObject* PyCast::toPyObject_impl<NamedHash>(const NamedHash& obj)
{
    return toPyObject<sp<NamedHash>>(sp<NamedHash>::make(obj));
}

}
