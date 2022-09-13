#include "python/impl/duck/py_numeric_duck_type.h"

#include "core/inf/variable.h"

#include "python/extension/py_instance.h"
#include "python/extension/py_cast.h"

namespace ark {
namespace plugin {
namespace python {

PyNumericDuckType::PyNumericDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyNumericDuckType::to(sp<Integer>& inst)
{
    inst = PyCast::ensureCppObject<sp<Integer>>(_instance.pyObject());
}

void PyNumericDuckType::to(sp<Numeric>& inst)
{
    inst = PyCast::ensureCppObject<sp<Numeric>>(_instance.pyObject());
}

}
}
}
