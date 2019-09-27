#include "box2d/impl/shapes/ball.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

#include "graphics/base/size.h"

#include "box2d/impl/body_create_info.h"

namespace ark {
namespace plugin {
namespace box2d {

void Ball::apply(b2Body* body, const sp<Size>& size, const BodyCreateInfo& createInfo)
{
    b2CircleShape shape;

    DWARN(size->width() == size->height(), "RigidBody size: (%.2f, %.2f) is not a circle", size->width(), size->height());
    shape.m_radius = (size->width() + size->height()) / 4.0f;

    b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
    body->CreateFixture(&fixtureDef);
}

Ball::BUILDER::BUILDER()
{
}

sp<Shape> Ball::BUILDER::build(const Scope& args)
{
    return sp<Ball>::make();
}

}
}
}
