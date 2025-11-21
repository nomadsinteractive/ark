#include "box2d/impl/shapes/polygon.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

#include "graphics/components/size.h"
#include "graphics/base/v2.h"

#include "box2d/impl/body_create_info.h"

namespace ark::plugin::box2d {

Polygon::Polygon(const std::vector<V2>& vertices)
    : _vertices(vertices)
{
}

void Polygon::apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo)
{
    b2PolygonShape shape;
    b2Vec2* vecs = new b2Vec2[_vertices.size()];
    for(uint32_t i = 0; i < _vertices.size(); i ++)
        vecs[i].Set(_vertices[i].x() * size.x() / 2.0f, _vertices[i].y() * size.y() / 2.0f);
    shape.Set(vecs, static_cast<int32_t>(_vertices.size()));
    delete[] vecs;

    b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
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

sp<Shape> Polygon::BUILDER::build(const Scope& /*args*/)
{
    return sp<Polygon>::make(_vertices);
}

}
