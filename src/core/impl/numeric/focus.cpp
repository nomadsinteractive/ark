#include "core/impl/numeric/focus.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

namespace ark {

Focus::Focus(const sp<Numeric>& target, float nearest, float farest, float value)
    : _target(target), _nearest(nearest), _farest(farest), _value(value)
{
}

float Focus::val()
{
    float fv = _target->val();
    float d = fv - _value;
    if(d < _nearest)
        _value = fv - _nearest;
    else if(d > _farest)
        _value = fv - _farest;
    return _value;
}

Focus::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _target(factory.ensureBuilder<Numeric>(manifest, "target")), _nearest(factory.ensureBuilder<Numeric>(manifest, "nearest")),
      _farest(factory.ensureBuilder<Numeric>(manifest, "farest")), _value(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::VALUE))
{
}

sp<Numeric> Focus::BUILDER::build(const Scope& args)
{
    return sp<Numeric>::adopt(new Focus(_target->build(args), _nearest->build(args)->val(), _farest->build(args)->val(), _value->build(args)->val()));
}

}
