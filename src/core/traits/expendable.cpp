#include "core/traits/expendable.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Expendable::Expendable(bool discarded)
    : _discarded(sp<BooleanWrapper>::make(discarded))
{
}

Expendable::Expendable(sp<Boolean> discarded)
    : _discarded(sp<BooleanWrapper>::make(discarded))
{
}

bool Expendable::val()
{
    return _discarded->val();
}

bool Expendable::update(uint64_t timestamp)
{
    return _discarded->update(timestamp);
}

void Expendable::discard()
{
    _discarded->set(true);
}

void Expendable::set(bool discarded)
{
    _discarded->set(discarded);
}

void Expendable::set(sp<Boolean> discarded)
{
    _discarded->set(std::move(discarded));
}

Expendable::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _discarded(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = factory.ensureBuilder<Boolean>(value);
}

sp<Expendable> Expendable::DICTIONARY::build(const Scope& args)
{
    return _delegate ? sp<Expendable>::make(_delegate->build(args)) : sp<Expendable>::make(_discarded);
}

}
