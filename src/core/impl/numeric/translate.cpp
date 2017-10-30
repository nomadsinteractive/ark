#include "core/impl/numeric/translate.h"

#include "core/util/bean_utils.h"

namespace ark {

Translate::Translate(const sp<Numeric>& delegate, float translation)
    : _delegate(delegate), _translation(translation)
{
    NOT_NULL(_delegate);
}

float Translate::val()
{
    return _delegate->val() + _translation;
}

Translate::DECORATOR::DECORATOR(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value)
    : _delegate(delegate), _translation(parent.getBuilder<Numeric>(value))
{
}

sp<Numeric> Translate::DECORATOR::build(const sp<Scope>& args)
{
    return sp<Numeric>::adopt(new Translate(_delegate->build(args), BeanUtils::toFloat(_translation, args)));
}

}
