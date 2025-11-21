#include "box2d/impl/shapes/arc.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "box2d/impl/body_create_info.h"

namespace ark::plugin::box2d {

Arc::Arc(uint32_t sampleCount, float a, float b)
    : _sample_count(sampleCount), _a(a), _b(b)
{
}

void Arc::apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo)
{
    DCHECK_WARN(Math::almostEqual<float>(size.x(), size.y()), "RigidBody size: (%.2f, %.2f) is not a circle", size.x(), size.y());

    // b2ChainShape shape;
    // float radius = (size.x() + size.y()) / 4.0f;
    //
    // if(_b < _a)
    //     _b += 360.0f;
    //
    // std::vector<b2Vec2> vecs(_sample_count + 1);
    // float step = (_b - _a) * Math::PI / 180.0f / static_cast<float>(_sample_count);
    // float da = _a / 180 * Math::PI;
    // for(uint32_t i = 0; i <= _sample_count; i ++)
    // {
    //     float degree = da + step * static_cast<float>(i);
    //     vecs[i].Set(Math::cos(degree) * radius, Math::sin(degree) * radius);
    // }
    // shape.CreateLoop(vecs.data(), _sample_count + 1);
    //
    // b2FixtureDef fixtureDef = createInfo.toFixtureDef(&shape);
    // body->CreateFixture(&fixtureDef);
}

Arc::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _sample_count(factory.ensureBuilder<Numeric>(manifest, "sample-count")),
      _a(factory.ensureBuilder<Numeric>(manifest, "a")),
      _b(factory.ensureBuilder<Numeric>(manifest, "b"))
{
}

sp<Shape> Arc::BUILDER::build(const Scope& args)
{
    uint32_t sampleCount = static_cast<uint32_t>(_sample_count->build(args)->val());
    float a = _a->build(args)->val();
    float b = _b->build(args)->val();
    return sp<Arc>::make(sampleCount, a, b);
}

}
