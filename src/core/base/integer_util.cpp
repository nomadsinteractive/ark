#include "core/base/integer_util.h"

#include "core/impl/integer/integer_add.h"
#include "core/impl/integer/integer_floor_div.h"
#include "core/impl/integer/integer_multiply.h"
#include "core/impl/integer/integer_subtract.h"

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

void IntegerUtil::iadd(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    FATAL("Unimplemented");
}

sp<Integer> IntegerUtil::sub(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<IntegerSubtract>::make(self, rvalue);
}

void IntegerUtil::isub(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    FATAL("Unimplemented");
}

sp<Integer> IntegerUtil::mul(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<IntegerMultiply>::make(self, rvalue);
}

void IntegerUtil::imul(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    FATAL("Unimplemented");
}

sp<Numeric> IntegerUtil::truediv(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Integer> IntegerUtil::floordiv(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<IntegerFloorDiv>::make(self, rvalue);
}

sp<Integer> IntegerUtil::negative(const sp<Integer>& self)
{
    FATAL("Unimplemented");
    return nullptr;
}

int32_t IntegerUtil::toInt32(const sp<Integer>& self)
{
    return self->val();
}

int32_t IntegerUtil::val(const sp<Integer>& self)
{
    return self->val();
}

void IntegerUtil::setVal(const sp<Integer>& self, int32_t value)
{
    const sp<Integer::Impl> inst1 = self.as<Integer::Impl>();
    if(inst1)
    {
        inst1->set(value);
        return;
    }
    const sp<IntegerWrapper> inst2 = self.as<IntegerWrapper>();
    NOT_NULL(inst2);
    inst2->set(value);
}

const sp<Integer>& IntegerUtil::delegate(const sp<Integer>& self)
{
    DCHECK(self.is<IntegerWrapper>(), "Must be an IntegerWrapper instance to get its delegate attribute");
    return self.as<IntegerWrapper>()->delegate();
}

void IntegerUtil::setDelegate(const sp<Integer>& self, const sp<Integer>& delegate)
{
    DCHECK(self.is<IntegerWrapper>(), "Must be an IntegerWrapper instance to set its delegate attribute");
    self.as<IntegerWrapper>()->setDelegate(delegate);
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

void IntegerUtil::fix(const sp<Integer>& self)
{
    const sp<IntegerWrapper> iw = self.as<Integer>();
    DWARN(iw, "Calling fix on non-IntegerWrapper has no effect.");
    if(iw)
        iw->fix();
}

}
