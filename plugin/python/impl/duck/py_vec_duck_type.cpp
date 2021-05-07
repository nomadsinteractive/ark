#include "python/impl/duck/py_vec_duck_type.h"

#include "python/extension/py_instance.h"
#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

PyVecDuckType::PyVecDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyVecDuckType::to(sp<Vec2>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Vec2>>(_instance.pyObject());
}

void PyVecDuckType::to(sp<Vec3>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Vec3>>(_instance.pyObject());
}

void PyVecDuckType::to(sp<Vec4>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Vec4>>(_instance.pyObject());
}

void PyVecDuckType::to(sp<Rotation>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Rotation>>(_instance.pyObject());
}

}
}
}
