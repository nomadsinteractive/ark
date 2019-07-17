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

bool Visibility::val()
{
    return _visible->val();
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

Visibility::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _visible(factory.ensureBuilder<Boolean>(value))
{
}

sp<Visibility> Visibility::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Visibility>::make(_visible->build(args));
}

template<> ARK_API sp<Visibility> Null::ptr()
{
    return Ark::instance().obtain<Visibility>(true);
}

}
