#include "core/epi/visibility.h"

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

sp<Boolean> Visibility::toBoolean() const
{
    return _visible;
}

bool Visibility::visible() const
{
    return _visible->val();
}

void Visibility::setVisible(bool visible)
{
    _visible->set(visible);
}

Visibility::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _visible(value == "true")
{
    if(value && (value.at(0) == '@' || value.at(0) == '$'))
        _delegate = factory.ensureBuilder<Boolean>(value);
}

sp<Visibility> Visibility::DICTIONARY::build(const sp<Scope>& args)
{
    return _delegate ? sp<Visibility>::make(_delegate->build(args)) : sp<Visibility>::make(_visible);
}

}
