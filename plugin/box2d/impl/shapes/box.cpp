#include "box2d/impl/shapes/box.h"

#include "core/base/bean_factory.h"

#include "graphics/components/size.h"

#include "box2d/impl/body_create_info.h"

namespace ark::plugin::box2d {

void Box::apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo)
{
    b2Polygon polygon = b2MakeBox(size.x() / 2.0f, size.y() / 2.0f);

    b2ShapeDef shapeDef = createInfo.toShapeDef();
    b2CreatePolygonShape(body, &shapeDef, &polygon);
}

Box::BUILDER::BUILDER()
{
}

sp<Shape> Box::BUILDER::build(const Scope& /*args*/)
{
    return sp<Box>::make();
}

}
