#include "python/impl/duck/py_object_duck_type.h"

#include "core/impl/range/integer_array.h"

#include "graphics/base/color.h"

#include "python/extension/python_interpreter.h"
#include "python/impl/adapter/collision_callback_python_adapter.h"

namespace ark {
namespace plugin {
namespace python {

PyObjectDuckType::PyObjectDuckType(const sp<PyInstance>& inst)
    : _instance(inst)
{
}

void PyObjectDuckType::to(sp<CollisionCallback>& inst)
{
    inst = sp<CollisionCallbackPythonAdapter>::make(_instance);
}

void PyObjectDuckType::to(sp<Array<Color>>& inst)
{
    inst = PythonInterpreter::instance()->toArray<Color>(_instance->instance());
}

void PyObjectDuckType::to(sp<Array<int32_t>>& inst)
{
    inst = sp<IntegerArray>::make(PythonInterpreter::instance()->toArray<int32_t>(_instance->instance()));
}

}
}
}
