#include "graphics/components/shape.h"

#include "core/impl/variable/variable_dirty_mark.h"

namespace ark {

Shape::Shape(const NamedHash& type, Optional<V3> scale, const V3 origin)
    : Shape(type, std::move(scale), std::move(origin), {})
{
}

Shape::Shape(const NamedHash& type, Optional<V3> scale, const V3 orgin, Box implementation)
    : _type(type), _origin(orgin), _scale(std::move(scale)), _implementation(std::move(implementation))
{
}

const NamedHash& Shape::type() const
{
    return _type;
}

V3 Shape::origin() const
{
    return _origin;
}

const Optional<V3>& Shape::scale() const
{
    return _scale;
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
