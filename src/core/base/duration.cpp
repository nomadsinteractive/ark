#include "core/base/duration.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/util/math.h"

namespace ark {

Duration::Duration(const sp<Numeric>& delegate)
    : _delegate(delegate)
{
    DASSERT(delegate);
}

float Duration::val()
{
    return _delegate->val();
}

bool Duration::update(uint64_t timestamp)
{
    return _delegate->update(timestamp);
}

sp<Numeric> Duration::duration()
{
    return Ark::instance().appClock()->duration();
}

Duration::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _clock(factory.getBuilder<Clock>(manifest, Constants::Attributes::CLOCK)),
      _delegate(factory.getBuilder<Numeric>(manifest, "t")),
      _until(factory.getBuilder<Numeric>(manifest, "until"))
{
    DCHECK_WARN(!(_clock && _delegate), "Clock will be ommited since \"t\" is specified");
}

sp<Duration> Duration::BUILDER::build(const Scope& args)
{
    const sp<Numeric> util = _until ? _until->build(args) : nullptr;
    if(_delegate)
    {
        const sp<Numeric> delegate = _delegate->build(args);
        return sp<Duration>::make(util ? Math::min(delegate, util) : delegate);
    }
    const sp<Clock> clock = _clock ? _clock->build(args) : Ark::instance().appClock();
    return sp<Duration>::make(util ? clock->durationUntil(util) : clock->duration());
}

Duration::NUMERIC_BUILDER::NUMERIC_BUILDER(BeanFactory& factory, const document& manifest)
    : _builder(factory, manifest)
{
}

sp<Numeric> Duration::NUMERIC_BUILDER::build(const Scope& args)
{
    return _builder.build(args);
}

Duration::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/)
{
}

sp<Duration> Duration::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<Duration>::make(Ark::instance().appClock()->duration());
}

}
