#include "python/impl/duck/py_array_duck_type.h"

#include "graphics/base/color.h"

#include "python/extension/python_interpreter.h"
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
    inst = PythonInterpreter::instance()->toArray<sp<Mat3>>(_instance.pyObject());
}

void PyArrayDuckType::to(sp<Array<sp<Mat4>>>& inst)
{
    inst = PythonInterpreter::instance()->toArray<sp<Mat4>>(_instance.pyObject());
}

}
}
}
