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

void PyObjectDuckType::to(sp<String>& inst)
{
    inst = sp<String>::make(PythonInterpreter::instance()->toString(_instance.pyObject()));
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

void PyObjectDuckType::to(sp<Integer>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Integer>>(_instance.pyObject());
}

void PyObjectDuckType::to(sp<Numeric>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Numeric>>(_instance.pyObject());
}

}
}
}
