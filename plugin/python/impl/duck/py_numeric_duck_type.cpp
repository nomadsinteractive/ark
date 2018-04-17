#include "python/impl/duck/py_numeric_duck_type.h"

#include <Python.h>

#include "core/inf/variable.h"

#include "python/extension/py_instance.h"

namespace ark {
namespace plugin {
namespace python {

PyNumericDuckType::PyNumericDuckType(const sp<PyInstance>& inst)
    : _instance(inst)
{
}

void PyNumericDuckType::to(sp<Integer>& inst)
{
    inst = sp<Integer::Impl>::make(PyLong_AsLong(_instance->instance()));
}

void PyNumericDuckType::to(sp<Numeric>& inst)
{

    inst = sp<Numeric::Impl>::make(PyFloat_AsDouble(_instance->instance()));
}

}
}
}
