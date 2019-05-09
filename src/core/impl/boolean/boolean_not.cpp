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

}
