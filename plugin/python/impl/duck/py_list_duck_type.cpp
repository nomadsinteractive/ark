#include "python/impl/duck/py_list_duck_type.h"

#include "python/extension/py_cast.h"
#include "python/extension/python_extension.h"

namespace ark::plugin::python {

PyListDuckType::PyListDuckType(PyInstance instance)
    : _instance(std::move(instance))
{
}

void PyListDuckType::to(sp<Vec2>& inst)
{
    inst = PyCast::ensureSharedPtr<Vec2>(_instance.pyObject());
}

void PyListDuckType::to(sp<Vec3>& inst)
{
    inst = PyCast::ensureSharedPtr<Vec3>(_instance.pyObject());
}

void PyListDuckType::to(sp<Vec4>& inst)
{
    inst = PyCast::ensureSharedPtr<Vec4>(_instance.pyObject());
}

void PyListDuckType::to(sp<std::vector<Box>>& inst)
{
    const Py_ssize_t objSize = PyObject_Size(_instance.pyObject());
    const PythonExtension& pi = PythonExtension::instance();

    std::vector<Box> boxes;
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
    inst = sp<std::vector<Box>>::make(std::move(boxes));
}

}
