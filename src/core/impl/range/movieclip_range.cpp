#include "core/impl/range/movieclip_range.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"

namespace ark {

MovieclipRange::MovieclipRange(const sp<Range>& delegate, const sp<Numeric>& duration, float interval)
    : _delegate(delegate), _duration(duration), _interval(interval), _next_update(0), _has_next1(delegate->hasNext()), _has_next2(_has_next1)
{
}

bool MovieclipRange::hasNext()
{
    return _has_next2;
}

int32_t MovieclipRange::next()
{
    float t = _duration->val();
    if(t >= _next_update)
    {
        if(_has_next1)
        {
            _last_value = _delegate->next();
            _has_next1 = _delegate->hasNext();
        }
        else
            _has_next2 = false;
        _next_update = t + _interval;
    }
    return _last_value;
}

MovieclipRange::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Range>(manifest, Constants::Attributes::DELEGATE)), _duration(factory.getBuilder<Numeric>(manifest, "t", false)),
      _interval(Documents::ensureAttribute<Clock::Interval>(manifest, Constants::Attributes::INTERVAL).sec())
{
}

sp<Range> MovieclipRange::BUILDER::build(const sp<Scope>& args)
{
    return sp<Range>::adopt(new MovieclipRange(_delegate->build(args), _duration ? _duration->build(args) : Ark::instance().clock()->duration(), _interval));
}

}
