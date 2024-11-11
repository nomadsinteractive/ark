#include "app/traits/shape.h"

#include "core/impl/variable/variable_dirty.h"

namespace ark {

Shape::Shape(const NamedHash& type, sp<Vec3> size, sp<Vec3> orgin)
    : Shape(type, std::move(size), std::move(orgin), {})
{
}

Shape::Shape(const NamedHash& type, sp<Vec3> size, sp<Vec3> orgin, Box implementation)
    : _type(type), _size(std::move(size)), _orgin(std::move(orgin)), _implementation(std::move(implementation))
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

const SafeVar<Vec3>& Shape::orgin() const
{
    return _orgin;
}

}
