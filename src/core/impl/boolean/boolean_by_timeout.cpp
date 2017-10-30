#include "core/impl/boolean/boolean_by_timeout.h"

#include "core/ark.h"
#include "core/base/clock.h"
#include "core/base/bean_factory.h"
#include "core/util/bean_utils.h"
#include "core/util/documents.h"

namespace ark {

BooleanByTimeout::BooleanByTimeout(const sp<Variable<uint64_t>>& ticker, uint64_t timeout)
    : _ticker(ticker), _timeout(ticker->val() + timeout)
{
}

bool BooleanByTimeout::val()
{
    return _ticker->val() > _timeout;
}

BooleanByTimeout::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _usec(0)
{
    const String& timeout = Documents::ensureAttribute(doc, Constants::Attributes::TIMEOUT);
    if(timeout.at(0) == '$' || timeout.at(0) == '@')
        _timeout = parent.ensureBuilder<Numeric>(timeout);
    else
    {
        _usec = Strings::parse<Clock::Interval>(timeout).usec();
        DCHECK(_usec, "Timeout value must not be zero");
    }
}

sp<Boolean> BooleanByTimeout::BUILDER::build(const sp<Scope>& args)
{
    if(!_usec)
    {
        _usec = static_cast<uint64_t>(BeanUtils::toFloat(_timeout, args, 0.0f) * 1000000);
        DCHECK(_usec, "Timeout value must not be zero");
        _timeout = nullptr;
    }
    return sp<Boolean>::adopt(new BooleanByTimeout(Ark::instance().clock(), static_cast<uint64_t>(_usec)));
}

}
