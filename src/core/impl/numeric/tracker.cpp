#include "core/impl/numeric/tracker.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/inf/runnable.h"
#include "core/util/math.h"

namespace ark {

Tracker::Tracker(const sp<Numeric>& delegate, const sp<Runnable>& oncross, float pitch)
    : _delegate(delegate), _oncross(oncross), _pitch(pitch), _floor(Math::modFloor(_delegate->val(), pitch))
{
}

float Tracker::val()
{
    float v = _delegate->val();
    if(v >= _floor && v < _floor + _pitch)
        return v;
    _floor = Math::modFloor(v, _pitch);
    _oncross->run();
    return v;
}

Tracker::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::DELEGATE)), _pitch(factory.ensureBuilder<Numeric>(manifest, "pitch")),
      _oncross(factory.ensureBuilder<Runnable>(manifest, Constants::Attributes::ON_CROSS))
{
}

sp<Numeric> Tracker::BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Tracker(_delegate->build(args), _oncross->build(args), _pitch->build(args)->val()));
}

}
