#include "python/impl/duck/py_list_duck_type.h"

#include "graphics/util/vec2_type.h"
#include "graphics/util/vec3_type.h"

#include "python/extension/py_cast.h"
#include "python/extension/python_extension.h"

namespace ark::plugin::python {

PyListDuckType::PyListDuckType(PyInstance instance)
    : _instance(std::move(instance))
{
}

void PyListDuckType::to(sp<Vec2>& inst)
{
    if(Optional<sp<Vec2>> opt = PyCast::toSharedPtr<Vec2>(_instance.pyObject()))
        inst = std::move(opt.value());
}

void PyListDuckType::to(sp<Vec3>& inst)
{
    if(Optional<sp<Vec3>> opt = PyCast::toSharedPtr<Vec3>(_instance.pyObject()))
        inst = std::move(opt.value());
}

void PyListDuckType::to(sp<Vec4>& inst)
{
    if(Optional<sp<Vec4>> opt = PyCast::toSharedPtr<Vec4>(_instance.pyObject()))
        inst = std::move(opt.value());
}

void PyListDuckType::to(sp<Size>& inst)
{
    const Py_ssize_t size = PyObject_Size(_instance.pyObject());
    if(size != 2 && size != 3)
        return;
    inst = size == 2 ? Vec2Type::toSize(PyCast::ensureSharedPtr<Vec2>(_instance.pyObject())) : Vec3Type::toSize(PyCast::ensureSharedPtr<Vec3>(_instance.pyObject()));
}

void PyListDuckType::to(sp<Vector<Box>>& inst)
{
    const Py_ssize_t objSize = PyObject_Size(_instance.pyObject());
    const PythonExtension& pi = PythonExtension::instance();

    Vector<Box> boxes;
    for(Py_ssize_t i = 0; i < objSize; ++i)
    {
        PyObject* key = PyLong_FromSsize_t(i);
        PyObject* item = PyObject_GetItem(_instance.pyObject(), key);
        if(Py_IsNone(item))
            boxes.push_back({});
        else
        {
            ASSERT(pi.isPyArkTypeObject(Py_TYPE(item)));
            boxes.push_back(*reinterpret_cast<PyArkType::Instance*>(item)->box);
        }
        Py_XDECREF(item);
        Py_XDECREF(key);
    }
    inst = sp<Vector<Box>>::make(std::move(boxes));
}

}
