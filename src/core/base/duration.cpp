#include "core/base/duration.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/impl/numeric/min.h"

namespace ark {

Duration::Duration(const sp<Numeric>& delegate)
    : _delegate(delegate)
{
    NOT_NULL(delegate);
}

float Duration::val()
{
    return _delegate->val();
}

Duration::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _clock(factory.getBuilder<Clock>(manifest, Constants::Attributes::CLOCK, false)),
      _delegate(factory.getBuilder<Numeric>(manifest, "t", false)),
      _util(factory.getBuilder<Numeric>(manifest, "util", false))
{
    DWARN(!(_clock && _delegate), "Clock will be ommited since \"t\" is specified");
}

sp<Duration> Duration::BUILDER::build(const sp<Scope>& args)
{
    const sp<Numeric> util = _util ? _util->build(args) : sp<Numeric>::null();
    if(_delegate)
    {
        const sp<Numeric> delegate = _delegate->build(args);
        return sp<Duration>::make(util ? sp<Numeric>::adopt(new Min(delegate, util)) : delegate);
    }
    const sp<Clock> clock = _clock ? _clock->build(args) : Ark::instance().clock();
    return sp<Duration>::make(util ? clock->durationUtil(util) : clock->duration());
}

Duration::NUMERIC_BUILDER::NUMERIC_BUILDER(BeanFactory& factory, const document& manifest)
    : _builder(factory, manifest)
{
}

sp<Numeric> Duration::NUMERIC_BUILDER::build(const sp<Scope>& args)
{
    return _builder.build(args);
}

Duration::DICTIONARY::DICTIONARY(BeanFactory& /*factory*/)
{
}

sp<Duration> Duration::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<Duration>::make(Ark::instance().clock()->duration());
}

}
