#include "core/impl/integer/integer_impl.h"

namespace ark {

IntegerImpl::IntegerImpl(const sp<Integer>& delegate)
    : _value(0), _delegate(delegate)
{
}

IntegerImpl::IntegerImpl(int32_t value)
    : _value(value)
{
}

int32_t IntegerImpl::val()
{
    return _delegate ? _delegate->val() : _value;
}

}
