#include "box2d/impl/shapes/box.h"

#include "core/base/bean_factory.h"

#include "graphics/base/size.h"

#include "box2d/impl/body_create_info.h"

namespace ark {
namespace plugin {
namespace box2d {

void Box::apply(b2Body* body, const sp<Size>& size, const BodyCreateInfo& createInfo)
{
    b2PolygonShape shape;
    shape.SetAsBox(size->width() / 2.0f, size->height() / 2.0f);

    b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
    body->CreateFixture(&fixtureDef);
}

Box::BUILDER::BUILDER()
{
}

sp<Shape> Box::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<Box>::make();
}

}
}
}
