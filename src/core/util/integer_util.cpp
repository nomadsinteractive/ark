#include "core/util/integer_util.h"

#include "core/base/bean_factory.h"
#include "core/base/variable_wrapper.h"
#include "core/impl/integer/integer_add.h"
#include "core/impl/integer/integer_floor_div.h"
#include "core/impl/integer/integer_multiply.h"
#include "core/impl/integer/integer_subtract.h"
#include "core/util/strings.h"

namespace ark {

sp<Integer> IntegerUtil::create(const sp<Integer>& value)
{
    return sp<IntegerWrapper>::make(value);
}

sp<Integer> IntegerUtil::create(int32_t value)
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

sp<Integer> IntegerUtil::mod(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
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

float IntegerUtil::toFloat(const sp<Integer>& self)
{
    return static_cast<float>(self->val());
}

int32_t IntegerUtil::val(const sp<Integer>& self)
{
    return self->val();
}

void IntegerUtil::setVal(const sp<Integer::Impl>& self, int32_t value)
{
    self->set(value);
}

void IntegerUtil::setVal(const sp<IntegerWrapper>& self, int32_t value)
{
    self->set(value);
}

const sp<Integer>& IntegerUtil::delegate(const sp<Integer>& self)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DCHECK(iw, "Must be an IntegerWrapper instance to get its delegate attribute");
    return iw->delegate();
}

void IntegerUtil::setDelegate(const sp<Integer>& self, const sp<Integer>& delegate)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DCHECK(iw, "Must be an IntegerWrapper instance to set its delegate attribute");
    iw->set(delegate);
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
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DWARN(iw, "Calling fix on non-IntegerWrapper has no effect.");
    if(iw)
        iw->fix();
}

IntegerUtil::DICTIONARY::DICTIONARY(BeanFactory& /*beanFactory*/, const String& value)
    : _value(Strings::parse<int32_t>(value))
{
}

sp<Integer> IntegerUtil::DICTIONARY::build(const sp<Scope>& args)
{
    return sp<Integer::Impl>::make(_value);
}

IntegerUtil::ARRAY_DICTIONARY::ARRAY_DICTIONARY(BeanFactory& factory, const String& value)
{
    for(const String i : Strings::unwrap(value,'[', ']').split(','))
        _array_builders.push_back(factory.ensureBuilder<Integer>(i));
}

sp<IntArray> IntegerUtil::ARRAY_DICTIONARY::build(const sp<Scope>& args)
{
    const sp<IntArray> s = sp<DynamicArray<int32_t>>::make(_array_builders.size());
    int32_t* buf = s->buf();
    for(size_t i = 0; i < _array_builders.size(); i++)
        buf[i] = _array_builders[i]->build(args)->val();
    return s;
}

}
