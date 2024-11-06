#include "app/traits/shape.h"

#include "core/impl/variable/variable_dirty.h"

namespace ark {

Shape::Shape(const NamedHash& type, sp<Vec3> size)
    : Shape(type, std::move(size), {})
{
}

Shape::Shape(const NamedHash& type, sp<Vec3> size, Box implementation)
    : _type(type), _size(std::move(size)), _implementation(std::move(implementation))
{
}

const NamedHash& Shape::type() const
{
    return _type;
}

const SafeVar<Vec3>& Shape::size() const
{
    return _size;
}

}
