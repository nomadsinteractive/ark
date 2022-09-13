#include "python/impl/duck/py_vec_duck_type.h"

#include "python/extension/py_instance.h"
#include "python/extension/py_cast.h"

namespace ark {
namespace plugin {
namespace python {

PyVecDuckType::PyVecDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyVecDuckType::to(sp<Vec2>& inst)
{
    inst = PyCast::ensureCppObject<sp<Vec2>>(_instance.pyObject());
}

void PyVecDuckType::to(sp<Vec3>& inst)
{
    inst = PyCast::ensureCppObject<sp<Vec3>>(_instance.pyObject());
}

void PyVecDuckType::to(sp<Size>& inst)
{
    inst = PyCast::ensureCppObject<sp<Size>>(_instance.pyObject());
}

void PyVecDuckType::to(sp<Vec4>& inst)
{
    inst = PyCast::ensureCppObject<sp<Vec4>>(_instance.pyObject());
}

void PyVecDuckType::to(sp<Rotation>& inst)
{
    inst = PyCast::ensureCppObject<sp<Rotation>>(_instance.pyObject());
}

}
}
}
