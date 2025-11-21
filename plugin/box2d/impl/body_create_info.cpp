#include "box2d/impl/body_create_info.h"

namespace ark::plugin::box2d {

BodyCreateInfo::BodyCreateInfo()
    : density(0), friction(0), category(0), mask(0), group(0)
{
}

BodyCreateInfo::BodyCreateInfo(const sp<Shape>& shape, const float density, const float friction, const bool isSensor)
    : shape(shape), density(density), friction(friction), is_sensor(isSensor), category(0), mask(0), group(0)
{
}

b2ShapeDef BodyCreateInfo::toFixtureDef(const b2ShapeId shape) const
{
    b2ShapeDef fixture = b2DefaultShapeDef();
    // fixture.shape = shape;
    fixture.density = density;
    // fixture.friction = friction;
    fixture.isSensor = is_sensor;
    return fixture;
}

}
