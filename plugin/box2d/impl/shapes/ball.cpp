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

    DCHECK_WARN(size->widthAsFloat() == size->heightAsFloat(), "RigidBody size: (%.2f, %.2f) is not a circle", size->widthAsFloat(), size->heightAsFloat());
    shape.m_radius = (size->widthAsFloat() + size->heightAsFloat()) / 4.0f;

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
