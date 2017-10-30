#include "box2d/impl/shapes/box.h"

#include "core/base/bean_factory.h"

#include "graphics/base/size.h"

namespace ark {
namespace plugin {
namespace box2d {

Box::Box(float width, float height)
    : _width(width), _height(height)
{
}

void Box::apply(b2Body* body, float density, float friction)
{
    b2PolygonShape shape;
    shape.SetAsBox(_width / 2.0f, _height / 2.0f);

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    body->CreateFixture(&fixtureDef);
}

Box::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _size(parent.ensureBuilder<Size>(doc, Constants::Attributes::SIZE))
{
}

sp<Shape> Box::BUILDER::build(const sp<Scope>& args)
{
    const sp<Size> size = _size->build(args);
    return sp<Box>::make(size->width(), size->height());
}

}
}
}
