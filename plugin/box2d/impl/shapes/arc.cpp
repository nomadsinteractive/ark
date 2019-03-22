#include "box2d/impl/shapes/arc.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/size.h"

#include "box2d/impl/body_create_info.h"

namespace ark {
namespace plugin {
namespace box2d {

Arc::Arc(uint32_t sampleCount, float a, float b)
    : _sample_count(sampleCount), _a(a), _b(b)
{
}

void Arc::apply(b2Body* body, const sp<Size>& size, const BodyCreateInfo& createInfo)
{
    b2ChainShape shape;

    DWARN(size->width() == size->height(), "RigidBody size: (%.2f, %.2f) is not a circle", size->width(), size->height());

    float radius = (size->width() + size->height()) / 4.0f;

    if(_b < _a)
        _b += 360.0f;

    b2Vec2* vecs = new b2Vec2[_sample_count + 1];
    float step = (_b - _a) / _sample_count * Math::PI / 180;
    float da = _a / 180 * Math::PI;
    for(uint32_t i = 0; i <= _sample_count; i ++)
    {
        float degree = da + step * i;
        vecs[i].Set(Math::cos(degree) * radius, Math::sin(degree) * radius);
    }
    shape.CreateChain(vecs, _sample_count + 1);
    delete[] vecs;

    b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
    body->CreateFixture(&fixtureDef);
}

Arc::BUILDER::BUILDER(BeanFactory& parent, const document& manifest)
    : _sample_count(parent.ensureBuilder<Numeric>(manifest, "sample-count")),
      _a(parent.ensureBuilder<Numeric>(manifest, "a")),
      _b(parent.ensureBuilder<Numeric>(manifest, "b"))
{
}

sp<Shape> Arc::BUILDER::build(const sp<Scope>& args)
{
    uint32_t sampleCount = static_cast<uint32_t>(_sample_count->build(args)->val());
    float a = _a->build(args)->val();
    float b = _b->build(args)->val();
    return sp<Arc>::make(sampleCount, a, b);
}

}
}
}
