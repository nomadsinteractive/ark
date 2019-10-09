#include "python/impl/duck/py_object_duck_type.h"

#include "graphics/base/color.h"

#include "python/extension/python_interpreter.h"
#include "python/impl/adapter/collision_callback_python_adapter.h"
#include "python/impl/adapter/renderer_maker_python.h"

namespace ark {
namespace plugin {
namespace python {

PyObjectDuckType::PyObjectDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyObjectDuckType::to(sp<CollisionCallback>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<CollisionCallback>>(_instance.pyObject());
}

void PyObjectDuckType::to(sp<RendererMaker>& inst)
{
    PyInstance recycler = _instance.hasAttr("recycle") ? _instance.getAttr("recycle") : PyInstance();
    inst = sp<RendererMakerPython>::make(_instance.getAttr("make"), std::move(recycler));
}

void PyObjectDuckType::to(sp<Array<Color>>& inst)
{
    inst = PythonInterpreter::instance()->toArray<Color>(_instance.pyObject());
}

void PyObjectDuckType::to(sp<Array<int32_t>>& inst)
{
    inst = PythonInterpreter::instance()->toArray<int32_t>(_instance.pyObject());
}

}
}
}
