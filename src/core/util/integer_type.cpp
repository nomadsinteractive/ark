#include "core/util/integer_type.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/expression.h"
#include "core/impl/integer/integer_by_array.h"
#include "core/impl/variable/at_least.h"
#include "core/impl/variable/at_most.h"
#include "core/impl/variable/clamp.h"
#include "core/impl/variable/fence.h"
#include "core/impl/variable/periodic.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/string_convert.h"
#include "core/util/operators.h"
#include "core/util/strings.h"
#include "core/util/shared_ptr_util.h"

namespace ark {

namespace {

class IntegerArray {
public:
    class BUILDER : public Builder<Integer> {
    public:
        BUILDER(std::vector<int32_t> values, IntegerType::Repeat repeat)
            : _values(std::move(values)), _repeat(repeat) {
        }

        virtual sp<Integer> build(const Scope& /*args*/) override {
            return sp<IntegerByArray>::make(sp<IntArray::Vector>::make(_values), _repeat, Notifier());
        }

    private:
        std::vector<int32_t> _values;
        IntegerType::Repeat _repeat;
    };

};

static int32_t toInteger(float value) {
    return static_cast<int32_t>(value);
}

}

sp<IntegerWrapper> IntegerType::create(sp<Integer> value)
{
    return sp<IntegerWrapper>::make(std::move(value));
}

sp<IntegerWrapper> IntegerType::create(sp<Numeric> value)
{
    return sp<IntegerWrapper>::make(sp<VariableOP1<int32_t, float>>::make(toInteger, std::move(value)));
}

sp<IntegerWrapper> IntegerType::create(int32_t value)
{
    return sp<IntegerWrapper>::make(value);
}

sp<Integer> IntegerType::add(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Add<int32_t>>>::make(self, rvalue);
}

sp<Integer> IntegerType::sub(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Sub<int32_t>>>::make(self, rvalue);
}

sp<Integer> IntegerType::mul(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Mul<int32_t>>>::make(self, rvalue);
}

sp<Integer> IntegerType::mod(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Mod<int32_t>>>::make(self, rvalue);
}

sp<Numeric> IntegerType::truediv(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Div<float, int32_t>>>::make(self, rvalue);
}

sp<Integer> IntegerType::floordiv(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Div<int32_t>>>::make(self, rvalue);
}

sp<Integer> IntegerType::negative(sp<Integer> self)
{
    return sp<VariableOP1<int32_t>>::make(Operators::Neg<int32_t>(), std::move(self));
}

int32_t IntegerType::toInt32(const sp<Integer>& self)
{
    return self->val();
}

float IntegerType::toFloat(const sp<Integer>& self)
{
    return static_cast<float>(self->val());
}

sp<Boolean> IntegerType::gt(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::GT<int32_t>>>::make(self, other);
}

sp<Boolean> IntegerType::ge(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::GE<int32_t>>>::make(self, other);
}

sp<Boolean> IntegerType::lt(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::LT<int32_t>>>::make(self, other);
}

sp<Boolean> IntegerType::le(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::LE<int32_t>>>::make(self, other);
}

sp<Boolean> IntegerType::eq(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::EQ<int32_t>>>::make(self, other);
}

sp<Boolean> IntegerType::ne(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::NE<int32_t>>>::make(self, other);
}

int32_t IntegerType::val(const sp<Integer>& self)
{
    return self->val();
}

void IntegerType::setVal(const sp<Integer::Impl>& self, int32_t value)
{
    self->set(value);
}

void IntegerType::setVal(const sp<IntegerWrapper>& self, int32_t value)
{
    self->set(value);
}

const sp<Integer>& IntegerType::delegate(const sp<Integer>& self)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DWARN(iw, "Non-IntegerWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return iw ? iw->wrapped() : sp<Integer>::null();
}

void IntegerType::setDelegate(const sp<Integer>& self, const sp<Integer>& delegate)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DCHECK(iw, "Must be an IntegerWrapper instance to set its delegate attribute");
    iw->set(delegate);
}

void IntegerType::set(const sp<Integer::Impl>& self, int32_t value)
{
    self->set(value);
}

void IntegerType::set(const sp<IntegerWrapper>& self, int32_t value)
{
    self->set(value);
}

void IntegerType::set(const sp<IntegerWrapper>& self, const sp<Integer>& delegate)
{
    self->set(delegate);
}

void IntegerType::fix(const sp<Integer>& self)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DWARN(iw, "Calling fix on non-IntegerWrapper has no effect.");
    if(iw)
        iw->fix();
}

sp<Integer> IntegerType::wrap(const sp<Integer>& self)
{
    return sp<IntegerWrapper>::make(self);
}

int32_t IntegerType::toRepeat(const String& repeat)
{
    int32_t action = 0, flags = 0;
    for(const String& i : repeat.split('|'))
    {
        const String s = i.strip().toLower();
        if(s == "none")
            action = IntegerType::REPEAT_NONE;
        else if(s == "reverse")
            action = IntegerType::REPEAT_REVERSE;
        else if(s == "last")
            flags |= IntegerType::REPEAT_LAST;
        else if(s == "loop")
            flags |= IntegerType::REPEAT_LOOP;
        else
        {
            DCHECK(s == "notify", "Unknow Repeat: \"%s, supported enums are [\"none\", \"last\", \"reverse\", \"loop\", \"notify\"]", s.c_str());
            flags |= IntegerType::REPEAT_NOTIFY;
        }
    }
    return static_cast<IntegerType::Repeat>(action | flags);
}

sp<ExpectationI> IntegerType::repeat(std::vector<int32_t> array, IntegerType::Repeat repeat)
{
    Notifier notifier;
    sp<Integer> delegate = sp<IntegerByArray>::make(sp<IntArray::Vector>::make(std::move(array)), repeat, notifier);
    return sp<ExpectationI>::make(std::move(delegate), std::move(notifier));
}

sp<Integer> IntegerType::animate(const sp<Integer>& self, const sp<Numeric>& interval, const sp<Numeric>& duration)
{
    return sp<Periodic<int32_t>>::make(self, interval ? interval : sp<Numeric>::make<Numeric::Const>(1.0f / 24), duration ? duration : Ark::instance().appClock()->duration());
}

sp<ExpectationI> IntegerType::atLeast(sp<Integer> self, sp<Integer> a1)
{
    Notifier notifier;
    sp<Integer> delegate = sp<AtLeast<int32_t>>::make(std::move(self), std::move(a1), notifier);
    return sp<ExpectationI>::make(std::move(delegate), std::move(notifier));
}

sp<ExpectationI> IntegerType::atMost(sp<Integer> self, sp<Integer> a1)
{
    Notifier notifier;
    sp<Integer> delegate = sp<AtMost<int32_t>>::make(std::move(self), std::move(a1), notifier);
    return sp<ExpectationI>::make(std::move(delegate), std::move(notifier));
}

sp<ExpectationI> IntegerType::clamp(const sp<Integer>& self, const sp<Integer>& min, const sp<Integer>& max)
{
    DASSERT(self && min && max);
    Notifier notifier;
    return sp<ExpectationI>::make(sp<Clamp<int32_t>>::make(self, min, max, notifier), std::move(notifier));
}

sp<ExpectationI> IntegerType::fence(const sp<Integer>& self, const sp<Integer>& a1)
{
    DASSERT(self && a1);
    Notifier notifier;
    return sp<ExpectationI>::make(sp<Fence<int32_t>>::make(self, a1, notifier), std::move(notifier));
}

sp<Integer> IntegerType::ifElse(const sp<Integer>& self, const sp<Boolean>& condition, const sp<Integer>& negative)
{
    return sp<VariableTernary<int32_t>>::make(condition, self, negative);
}

IntegerType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : DICTIONARY(factory, value, REPEAT_NONE)
{
}

IntegerType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr, Repeat repeat)
    : _value(makeIntegerBuilder(factory, expr.strip(), repeat))
{
}

sp<Integer> IntegerType::DICTIONARY::build(const Scope& args)
{
    return _value->build(args);
}

sp<Builder<Integer>> IntegerType::DICTIONARY::makeIntegerBuilder(BeanFactory& factory, const String& expr, Repeat repeat) const
{
    DCHECK(expr, "Empty Integer expression");
    if(expr.at(0) == '[')
    {
        DCHECK(expr.at(expr.length() - 1) == ']', "Illegal IntArray expression");
        std::vector<int32_t> values;
        for(const String& i : expr.substr(1, expr.length() - 1).split(','))
            values.push_back(Strings::parse<int32_t>(i.strip()));
        return sp<IntegerArray::BUILDER>::make(std::move(values), repeat);
    }
    return Expression::Compiler<int32_t, NumericOperation<int32_t>>().compile(factory, expr);
}

IntegerType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory, Documents::ensureAttribute(manifest, Constants::Attributes::VALUE), Documents::getAttribute<Repeat>(manifest, "repeat", REPEAT_NONE))
{
}

sp<Integer> IntegerType::BUILDER::build(const Scope& args)
{
    return _delegate.build(args);
}

template<> ARK_API IntegerType::Repeat StringConvert::to<String, IntegerType::Repeat>(const String& str)
{
    return static_cast<IntegerType::Repeat>(IntegerType::toRepeat(str));
}

}
