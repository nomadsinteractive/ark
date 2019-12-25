#include "core/impl/boolean/boolean_by_timeout.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/duration.h"
#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"

namespace ark {

BooleanByTimeout::BooleanByTimeout(const sp<Numeric>& ticker, float timeout)
    : _duration(ticker), _timeout(ticker->val() + timeout)
{
}

bool BooleanByTimeout::val()
{
    return _duration->val() > _timeout;
}

bool BooleanByTimeout::update(uint64_t timestamp)
{
    return _duration->update(timestamp);
}

BooleanByTimeout::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _duration(factory.ensureBuilder<Duration>(manifest)), _sec(0)
{
    const String& timeout = Documents::ensureAttribute(manifest, Constants::Attributes::TIMEOUT);
    if(timeout.at(0) == '$' || timeout.at(0) == '@')
        _timeout = factory.ensureBuilder<Numeric>(timeout);
    else
    {
        _sec = Strings::parse<Clock::Interval>(timeout).sec();
        DCHECK(_sec, "Timeout value must not be zero");
    }
}

sp<Boolean> BooleanByTimeout::BUILDER::build(const Scope& args)
{
    if(_sec == 0)
    {
        _sec = BeanUtils::toFloat(_timeout, args, 0.0f);
        DCHECK(_sec > 0, "Timeout value must not be zero");
        _timeout = nullptr;
    }
    return sp<Boolean>::make<BooleanByTimeout>(_duration->build(args), _sec);
}

}
