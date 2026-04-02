#include "box2d/impl/shapes/ball.h"

#include "core/base/bean_factory.h"
#include "core/util/math.h"

#include "graphics/components/size.h"

#include "box2d/impl/body_create_info.h"

namespace ark::plugin::box2d {

void Ball::apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo)
{
    DCHECK_WARN(Math::almostEqual<float>(size.x(), size.y()), "RigidBody size: (%.2f, %.2f) is not a circle", size.x(), size.y());

    b2Circle circle;
    circle.center = {0, 0};
    circle.radius = (size.x() + size.y()) / 4.0f;

    b2ShapeDef shapeDef = createInfo.toShapeDef();
    b2CreateCircleShape(body, &shapeDef, &circle);
}

Ball::BUILDER::BUILDER()
{
}

sp<Shape> Ball::BUILDER::build(const Scope& args)
{
    return sp<Ball>::make();
}

}
