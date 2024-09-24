#include "app/traits/shape.h"

#include "core/base/string.h"
#include "core/impl/variable/variable_dirty.h"

namespace ark {

Shape::Shape(TypeId type, sp<Vec3> size)
    : _type(type), _size(std::move(size))
{
}

TypeId Shape::type() const
{
    return _type;
}

void Shape::setType(TypeId type)
{
    _type = type;
}

void Shape::setType(const String& type)
{
    _type = type.hash();
}

const SafeVar<Vec3>& Shape::size() const
{
    return _size;
}

void Shape::setSize(sp<Vec3> size)
{
    _size = sp<Vec3>::make<VariableDirty<V3>>(std::move(size), _size);
}

}
