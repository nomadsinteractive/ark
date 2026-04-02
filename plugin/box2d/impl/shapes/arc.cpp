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

    const float radius = (size.x() + size.y()) / 4.0f;

    float a = _a;
    float b = _b;
    if(b < a)
        b += 360.0f;

    std::vector<b2Vec2> vecs(_sample_count + 1);
    const float step = (b - a) * Math::PI / 180.0f / static_cast<float>(_sample_count);
    const float da = a / 180.0f * Math::PI;
    for(uint32_t i = 0; i <= _sample_count; i++)
    {
        const float angle = da + step * static_cast<float>(i);
        vecs[i] = {Math::cos(angle) * radius, Math::sin(angle) * radius};
    }

    b2ShapeDef shapeDef = createInfo.toShapeDef();
    for(uint32_t i = 0; i < _sample_count; i++)
    {
        b2Segment segment;
        segment.point1 = vecs[i];
        segment.point2 = vecs[i + 1];
        b2CreateSegmentShape(body, &shapeDef, &segment);
    }
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
