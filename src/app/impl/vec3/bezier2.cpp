#include "app/impl/vec3/bezier2.h"

#include "core/ark.h"
#include "core/inf/runnable.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"

#include "core/util/math.h"

#include "graphics/impl/vec/vec3_impl.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/render_controller.h"

namespace ark {

Bezier2::Bezier2(const sp<Numeric>& t, const sp<Numeric>& v, const sp<Vec2>& p1, const sp<Vec2>& p2, const sp<Vec2>& p3, const sp<Runnable>& onarrival)
    : _t(t), _v(v), _p1(p1), _p2(p2), _p3(p3), _on_arrival(onarrival), _last(p1->val()), _last_t(0), _p(0)
{
}

V3 Bezier2::val()
{
    const V2 p1 = _p1->val(), p2 = _p2->val(), p3 = _p3->val();

    if(_p >= 1.0f)
    {
        const sp<Runnable> onarrival = std::move(_on_arrival);
        if(onarrival)
            onarrival->run();
        return V3(p3.x(), p3.y(), Math::atan2(p3.y() - p2.y(), p3.x() - p2.x()));
    }

    float t = _t->val();
    float v = _v->val();
    float duration = t - _last_t;
    float s = v * duration;

    V2 q1 = p1, q2 = p2;

    do {
        float distance = Math::hypot(_last.x() - p3.x(), _last.y() - p3.y());
        float iter = s / distance * std::max(1.0f - _p, 0.001f);
        _p += iter;
        q1 = interpolate(p1, p2, _p);
        q2 = interpolate(p2, p3, _p);
        const V2 step = interpolate(q1, q2, _p);
        s -= Math::hypot(step.x() - _last.x(), step.y() - _last.y());
        _last = step;
    } while(s > v / 120);
    _last_t = t - s / v;
    return V3(_last.x(), _last.y(), Math::atan2(q2.y() - q1.y(), q2.x() - q1.x()));
}

bool Bezier2::update(uint64_t timestamp)
{
    return _t->update(timestamp);
}

V2 Bezier2::interpolate(const V2& p1, const V2& p2, float t)
{
    return (p2 - p1) * t + p1;
}

Bezier2::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _t(factory.getBuilder<Numeric>(manifest, "t")), _v(factory.ensureBuilder<Numeric>(manifest, "v")),
      _p1(factory.ensureBuilder<Vec2>(manifest, "p1")), _p2(factory.ensureBuilder<Vec2>(manifest, "p2")), _p3(factory.ensureBuilder<Vec2>(manifest, "p3")),
      _on_arrival(factory.getBuilder<Runnable>(manifest, "onarrival")), _resource_loader_context(resourceLoaderContext)
{
}

sp<Vec3> Bezier2::BUILDER::build(const Scope& args)
{
    const sp<Vec3> vv3 = sp<Bezier2>::make(_t ? _t->build(args) : Ark::instance().clock()->duration(), _v->build(args), _p1->build(args), _p2->build(args), _p3->build(args), _on_arrival->build(args));
    const sp<Vec3> synchronized = _resource_loader_context->synchronize<V3>(vv3);
    return synchronized;
}

}
