#include "box2d/impl/shapes/box.h"

#include "core/base/bean_factory.h"

#include "graphics/components/size.h"

#include "box2d/impl/body_create_info.h"

namespace ark::plugin::box2d {

void Box::apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo)
{
    b2PolygonShape shape;
    shape.SetAsBox(size.x() / 2.0f, size.y() / 2.0f);

    b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
    body->CreateFixture(&fixtureDef);
}

Box::BUILDER::BUILDER()
{
}

sp<Shape> Box::BUILDER::build(const Scope& /*args*/)
{
    return sp<Box>::make();
}

}
