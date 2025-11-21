#include "box2d/impl/shapes/ball.h"

#include "core/base/bean_factory.h"

#include "graphics/components/size.h"

#include "box2d/impl/body_create_info.h"

namespace ark::plugin::box2d {

void Ball::apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo)
{
    // b2CircleShape shape;
    //
    // DCHECK_WARN(size.x() == size.y(), "RigidBody size: (%.2f, %.2f) is not a circle", size.x() == size.y());
    // shape.m_radius = (size.x() + size.y()) / 4.0f;
    //
    // b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
    // body->CreateFixture(&fixtureDef);
}

Ball::BUILDER::BUILDER()
{
}

sp<Shape> Ball::BUILDER::build(const Scope& args)
{
    return sp<Ball>::make();
}

}
