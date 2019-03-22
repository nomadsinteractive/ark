#include "box2d/impl/body_create_info.h"

namespace ark {
namespace plugin {
namespace box2d {

BodyCreateInfo::BodyCreateInfo()
    : density(0), friction(0), category(0), mask(0), group(0)
{
}

BodyCreateInfo::BodyCreateInfo(const sp<Shape>& shape, float density, float friction, bool isSensor)
    : shape(shape), density(density), friction(friction), is_sensor(isSensor), category(0), mask(0), group(0)
{
}

b2FixtureDef BodyCreateInfo::toFixtureDef(const b2Shape* shape) const
{
    b2FixtureDef fixture;
    fixture.shape = shape;
    fixture.density = density;
    fixture.friction = friction;
    fixture.isSensor = is_sensor;
    return fixture;
}

}
}
}
