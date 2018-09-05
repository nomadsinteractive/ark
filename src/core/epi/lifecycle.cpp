#include "core/epi/lifecycle.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

Lifecycle::Lifecycle(bool expired)
    : _expired(expired)
{
}

Lifecycle::Lifecycle(const sp<Boolean>& delegate)
    : _expired(false), _delegate(delegate)
{
}

bool Lifecycle::val()
{
    return expired();
}

bool Lifecycle::expired() const
{
    return _expired || (_delegate && _delegate->val());
}

void Lifecycle::expire()
{
    _expired = true;
}

Lifecycle::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _expired(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = parent.ensureBuilder<Boolean>(value);
}

sp<Lifecycle> Lifecycle::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate ? sp<Lifecycle>::make(_delegate->build(args)) : sp<Lifecycle>::make(_expired);
}

}
