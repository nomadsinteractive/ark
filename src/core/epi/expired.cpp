#include "core/epi/expired.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

Expired::Expired(bool expired)
    : _expired(expired)
{
}

Expired::Expired(const sp<Boolean>& delegate)
    : _expired(false), _delegate(delegate)
{
}

bool Expired::val()
{
    return expired();
}

bool Expired::expired() const
{
    return _expired || (_delegate && _delegate->val());
}

void Expired::expire()
{
    _expired = true;
}

Expired::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _delegate(parent.ensureBuilder<Boolean>(doc))
{
}

sp<Expired> Expired::BUILDER::build(const sp<Scope>& args)
{
    return sp<Expired>::make(_delegate->build(args));
}

Expired::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _expired(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = parent.ensureBuilder<Boolean>(value);
}

sp<Expired> Expired::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate ? sp<Expired>::make(_delegate->build(args)) : sp<Expired>::make(_expired);
}

}
