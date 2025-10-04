#include "graphics/components/shape.h"

#include "core/impl/variable/variable_dirty_mark.h"

namespace ark {

Shape::Shape(const NamedHash& type, Optional<V3> scale, sp<Vec3> origin)
    : Shape(type, std::move(scale), std::move(origin), {})
{
}

Shape::Shape(const NamedHash& type, Optional<V3> scale, sp<Vec3> orgin, Box implementation)
    : _type(type), _scale(std::move(scale)), _origin(std::move(orgin)), _implementation(std::move(implementation))
{
}

const NamedHash& Shape::type() const
{
    return _type;
}

const Optional<V3>& Shape::scale() const
{
    return _scale;
}

const OptionalVar<Vec3>& Shape::origin() const
{
    return _origin;
}

const Box& Shape::implementation() const
{
    return _implementation;
}

void Shape::setImplementation(Box implementation)
{
    _implementation = std::move(implementation);
}

}
