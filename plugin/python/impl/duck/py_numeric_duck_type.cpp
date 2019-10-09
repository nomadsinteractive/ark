#include "python/impl/duck/py_numeric_duck_type.h"

#include <Python.h>

#include "core/inf/variable.h"

#include "python/extension/py_instance.h"
#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

PyNumericDuckType::PyNumericDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyNumericDuckType::to(sp<Integer>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Integer>>(_instance.pyObject());
}

void PyNumericDuckType::to(sp<Numeric>& inst)
{
    inst = PythonInterpreter::instance()->toCppObject<sp<Numeric>>(_instance.pyObject());
}

}
}
}
