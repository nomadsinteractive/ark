#include "core/impl/integer/integer_by_interval.h"

#include "core/ark.h"
#include "core/base/clock.h"

#include "core/base/bean_factory.h"
#include "core/base/duration.h"
#include "core/inf/array.h"

namespace ark {

IntegerByInterval::IntegerByInterval(const sp<Integer>& delegate, const sp<Numeric>& duration, const sp<Numeric>& interval)
    : _delegate(delegate), _duration(duration), _interval(interval), _value(delegate->val()), _next_update_time(duration->val() + interval->val())
{
}

int32_t IntegerByInterval::val()
{
    float d = _duration->val();
    if(d > _next_update_time)
    {
        _next_update_time = d + _interval->val();
        _value = _delegate->val();
    }
    return _value;
}

IntegerByInterval::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Integer>(manifest, Constants::Attributes::DELEGATE)),
      _duration(factory.ensureBuilder<Duration>(manifest)), _interval(factory.ensureBuilder<Numeric>(manifest, Constants::Attributes::INTERVAL))
{
}

sp<Integer> IntegerByInterval::BUILDER::build(const sp<Scope>& args)
{
    return sp<IntegerByInterval>::make(_delegate->build(args), _duration->build(args), _interval->build(args));
}

IntegerByInterval::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Integer>>& delegate, const String& value)
    : _delegate(delegate), _duration(factory.ensureBuilder<Duration>(value)), _interval(factory.ensureBuilder<Numeric>(value))
{
}

sp<Integer> IntegerByInterval::STYLE::build(const sp<Scope>& args)
{
    const sp<Integer> delegate = _delegate->build(args);
    return sp<IntegerByInterval>::make(delegate, _duration->build(args), _interval->build(args)).absorb(delegate);
}

}
