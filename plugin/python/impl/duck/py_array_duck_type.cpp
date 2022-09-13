#include "python/impl/duck/py_array_duck_type.h"

#include "graphics/base/color.h"

#include "python/extension/py_cast.h"
#include "python/impl/adapter/collision_callback_python_adapter.h"

namespace ark {
namespace plugin {
namespace python {

PyArrayDuckType::PyArrayDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyArrayDuckType::to(sp<Array<sp<Mat3>>>& inst)
{
    inst = PyCast::toSharedPtrArray<Mat3>(_instance.pyObject());
}

void PyArrayDuckType::to(sp<Array<sp<Mat4>>>& inst)
{
    inst = PyCast::toSharedPtrArray<Mat4>(_instance.pyObject());
}

void PyArrayDuckType::to(sp<Array<sp<Input>>>& inst)
{
    inst = PyCast::toSharedPtrArray<Input>(_instance.pyObject());
}

}
}
}
