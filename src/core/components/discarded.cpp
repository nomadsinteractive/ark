#include "core/components/discarded.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Discarded::Discarded(bool discarded)
    : _discarded(sp<BooleanWrapper>::make(discarded))
{
}

Discarded::Discarded(sp<Boolean> discarded)
    : _discarded(sp<BooleanWrapper>::make(discarded))
{
}

bool Discarded::val()
{
    return _discarded->val();
}

bool Discarded::update(uint64_t timestamp)
{
    return _discarded->update(timestamp);
}

void Discarded::discard()
{
    _discarded->set(true);
}

void Discarded::set(bool discarded)
{
    _discarded->set(discarded);
}

void Discarded::set(sp<Boolean> discarded)
{
    _discarded->set(std::move(discarded));
}

Discarded::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _discarded(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = factory.ensureBuilder<Boolean>(value);
}

sp<Discarded> Discarded::DICTIONARY::build(const Scope& args)
{
    return _delegate ? sp<Discarded>::make(_delegate->build(args)) : sp<Discarded>::make(_discarded);
}

}
