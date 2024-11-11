#include "core/traits/visibility.h"

#include "core/base/bean_factory.h"
#include "core/impl/variable/variable_wrapper.h"

namespace ark {

Visibility::Visibility(bool visible)
    : _visible(sp<BooleanWrapper>::make(visible))
{
}

Visibility::Visibility(const sp<Boolean>& visible)
    : _visible(sp<BooleanWrapper>::make(visible))
{
}

bool Visibility::val()
{
    return _visible->val();
}

bool Visibility::update(uint64_t timestamp)
{
    return _visible->update(timestamp);
}

void Visibility::show()
{
    _visible->set(true);
}

void Visibility::hide()
{
    _visible->set(false);
}

void Visibility::set(bool visible)
{
    _visible->set(visible);
}

void Visibility::set(const sp<Boolean>& visible)
{
    _visible->set(visible);
}

const sp<Boolean>& Visibility::delegate() const
{
    return _visible->wrapped();
}

Visibility::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _visible(factory.ensureBuilder<Boolean>(value))
{
}

sp<Visibility> Visibility::DICTIONARY::build(const Scope& args)
{
    return sp<Visibility>::make(_visible->build(args));
}

}
