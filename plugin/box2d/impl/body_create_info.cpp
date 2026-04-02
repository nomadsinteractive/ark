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

b2ShapeDef BodyCreateInfo::toShapeDef() const
{
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = density;
    shapeDef.material.friction = friction;
    shapeDef.isSensor = is_sensor;
    shapeDef.enableContactEvents = true;
    shapeDef.enableSensorEvents = is_sensor;
    return shapeDef;
}

}
