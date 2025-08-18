#include "app/components/shape.h"

#include "core/impl/variable/variable_dirty_mark.h"

namespace ark {

Shape::Shape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin)
    : Shape(type, std::move(size), std::move(origin), {})
{
}

Shape::Shape(const NamedHash& type, sp<Vec3> size, sp<Vec3> orgin, Box implementation)
    : _type(type), _size(std::move(size)), _origin(std::move(orgin)), _implementation(std::move(implementation))
{
}

const NamedHash& Shape::type() const
{
    return _type;
}

const OptionalVar<Vec3>& Shape::size() const
{
    return _size;
}

const OptionalVar<Vec3>& Shape::origin() const
{
    return _origin;
}

}
