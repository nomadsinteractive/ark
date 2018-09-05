#include "python/impl/duck/py_numeric_duck_type.h"

#include <Python.h>

#include "core/inf/variable.h"

#include "python/extension/py_instance.h"
#include "python/extension/python_interpreter.h"

namespace ark {
namespace plugin {
namespace python {

PyNumericDuckType::PyNumericDuckType(const sp<PyInstance>& inst)
    : _instance(inst)
{
}

void PyNumericDuckType::to(sp<Integer>& inst)
{
    inst = sp<Integer::Impl>::make(PyLong_AsLong(_instance->object()));
}

void PyNumericDuckType::to(sp<Numeric>& inst)
{
    inst = PythonInterpreter::instance()->toNumeric(_instance->object());
}

}
}
}
