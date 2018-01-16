#include "core/base/integer_util.h"

#include "core/impl/integer/integer_add.h"
#include "core/impl/integer/integer_subtract.h"
#include "core/impl/integer/integer_multiply.h"

namespace ark {

sp<Integer> IntegerUtil::create(const sp<Integer>& value)
{
    return sp<IntegerWrapper>::make(value);
}

sp<Integer> IntegerUtil::create(const int32_t value)
{
    return sp<IntegerWrapper>::make(value);
}

sp<Integer> IntegerUtil::add(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<IntegerAdd>::make(self, rvalue);
}

sp<Integer> IntegerUtil::sub(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<IntegerSubtract>::make(self, rvalue);
}

sp<Integer> IntegerUtil::mul(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<IntegerMultiply>::make(self, rvalue);
}

int32_t IntegerUtil::toInt32(const sp<Integer>& self)
{
    return self->val();
}

void IntegerUtil::set(const sp<Integer::Impl>& self, int32_t value)
{
    self->set(value);
}

void IntegerUtil::set(const sp<IntegerWrapper>& self, int32_t value)
{
    self->set(value);
}

void IntegerUtil::set(const sp<IntegerWrapper>& self, const sp<Integer>& delegate)
{
    self->set(delegate);
}

}
