#include "python/impl/duck/py_object_duck_type.h"

#include "graphics/base/color.h"

#include "python/extension/py_cast.h"

namespace ark::plugin::python {

PyObjectDuckType::PyObjectDuckType(PyInstance inst)
    : _instance(std::move(inst))
{
}

void PyObjectDuckType::to(sp<String>& inst)
{
    inst = sp<String>::make(PyCast::toString(_instance.pyObject()));
}

void PyObjectDuckType::to(sp<Integer>& inst)
{
    if(Optional<sp<Integer>> opt = PyCast::toCppObject<sp<Integer>>(_instance.pyObject()))
        inst = std::move(opt.value());
    else
        inst = sp<Integer>::make<Integer::Const>(PyCast::ensureCppObject<String>(_instance.pyObject()).hash());
}

void PyObjectDuckType::to(sp<Numeric>& inst)
{
    inst = PyCast::ensureCppObject<sp<Numeric>>(_instance.pyObject());
}

void PyObjectDuckType::to(sp<Box>& inst)
{
    inst = sp<Box>::make(PyCast::ensureCppObject<Box>(_instance.pyObject()));
}

}
