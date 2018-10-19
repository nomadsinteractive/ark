#include "core/impl/boolean/boolean_not.h"

#include "core/base/bean_factory.h"

namespace ark {

BooleanNot::BooleanNot(const sp<Boolean>& delegate)
    : _delegate(delegate)
{
    DASSERT(_delegate);
}

bool BooleanNot::val()
{
    return !_delegate->val();
}

BooleanNot::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory.ensureBuilder<Boolean>(manifest, Constants::Attributes::DELEGATE))
{
}

sp<Boolean> BooleanNot::BUILDER::build(const sp<Scope>& args)
{
    return sp<BooleanNot>::make(_delegate->build(args));
}

}
