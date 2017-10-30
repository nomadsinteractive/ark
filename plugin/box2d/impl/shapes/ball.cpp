#include "box2d/impl/shapes/ball.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {
namespace plugin {
namespace box2d {

Ball::Ball(float radius)
    : _radius(radius)
{
}

void Ball::apply(b2Body* body, float density, float friction)
{
    b2CircleShape shape;
    shape.m_radius = _radius;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    body->CreateFixture(&fixtureDef);
}

Ball::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _radius(parent.ensureBuilder<Numeric>(doc, "radius"))
{
}

sp<Shape> Ball::BUILDER::build(const sp<Scope>& args)
{
    return sp<Ball>::make(_radius->build(args)->val());
}

}
}
}
