#include "app/traits/shape.h"

#include "core/impl/variable/variable_dirty.h"

namespace ark {

Shape::Shape(int32_t id, sp<Vec3> size)
    : _id(id), _size(std::move(size))
{
}

int32_t Shape::id() const
{
    return _id;
}

void Shape::setId(int32_t id)
{
    _id = id;
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
