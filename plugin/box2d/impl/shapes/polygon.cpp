#include "box2d/impl/shapes/polygon.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/size.h"

namespace ark {
namespace plugin {
namespace box2d {

Polygon::Polygon(const std::vector<V2>& vertices)
    : _vertices(vertices)
{
}

void Polygon::apply(b2Body* body, const sp<Size>& size, float density, float friction)
{
    b2PolygonShape shape;
    b2Vec2* vecs = new b2Vec2[_vertices.size()];
    for(uint32_t i = 0; i < _vertices.size(); i ++)
        vecs[i].Set(_vertices[i].x() * size->width() / 2.0f, _vertices[i].y() * size->height() / 2.0f);
    shape.Set(vecs, _vertices.size());
    delete[] vecs;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    body->CreateFixture(&fixtureDef);
}

Polygon::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest)
{
    for(const document& i : manifest->children())
    {
        float x = Documents::ensureAttribute<float>(i, "x");
        float y = Documents::ensureAttribute<float>(i, "y");
        _vertices.push_back(V2(x, y));
    }
}

sp<Shape> Polygon::BUILDER::build(const sp<Scope>& /*args*/)
{
    return sp<Polygon>::make(_vertices);
}

}
}
}