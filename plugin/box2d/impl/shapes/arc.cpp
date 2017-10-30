#include "box2d/impl/shapes/arc.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

namespace ark {
namespace plugin {
namespace box2d {

Arc::Arc(float radius, uint32_t sampleCount, float a, float b)
    : _radius(radius), _sample_count(sampleCount), _a(a), _b(b)
{
}

void Arc::apply(b2Body* body, float density, float friction)
{
    b2ChainShape shape;

    if(_b < _a)
        _b += 360.0f;

    b2Vec2* vecs = new b2Vec2[_sample_count + 1];
    float step = (_b - _a) / _sample_count * Math::PI / 180;
    float da = _a / 180 * Math::PI;
    for(uint32_t i = 0; i <= _sample_count; i ++)
    {
        float degree = da + step * i;
        vecs[i].Set(Math::cos(degree) * _radius, Math::sin(degree) * _radius);
    }
    shape.CreateChain(vecs, _sample_count + 1);
    delete[] vecs;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    body->CreateFixture(&fixtureDef);
}

Arc::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _radius(parent.ensureBuilder<Numeric>(doc, "radius")),
      _sample_count(parent.ensureBuilder<Numeric>(doc, "sample-count")),
      _a(parent.ensureBuilder<Numeric>(doc, "a")),
      _b(parent.ensureBuilder<Numeric>(doc, "b"))
{
}

sp<Shape> Arc::BUILDER::build(const sp<Scope>& args)
{
    float radius = _radius->build(args)->val();
    uint32_t sampleCount = static_cast<uint32_t>(_sample_count->build(args)->val());
    float a = _a->build(args)->val();
    float b = _b->build(args)->val();
    return sp<Arc>::make(radius, sampleCount, a, b);
}

}
}
}
