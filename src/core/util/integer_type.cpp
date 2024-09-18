#include "core/util/integer_type.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/constants.h"
#include "core/base/expression.h"
#include "core/base/named_type.h"
#include "core/impl/integer/integer_by_array.h"
#include "core/impl/variable/at_least.h"
#include "core/impl/variable/at_most.h"
#include "core/impl/variable/clamp.h"
#include "core/impl/variable/fence.h"
#include "core/impl/variable/periodic.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/inf/array.h"
#include "core/util/string_convert.h"
#include "core/util/operators.h"
#include "core/util/strings.h"

namespace ark {

namespace {

class IntegerSubscribed : public Integer, Implements<IntegerSubscribed, Integer> {
public:
    IntegerSubscribed(std::vector<sp<Integer>> values, sp<Integer> index)
        : _values(std::move(values)), _index(std::move(index)) {
        CHECK(_values.size() > 0, "IntegerVector should have at least one component");
        updateSubscription();
    }

    virtual bool update(uint64_t timestamp) override {
        bool indexDirty = _index->update(timestamp);
        if(indexDirty)
            updateSubscription();
        return _subscribed->update(timestamp) || indexDirty;
    }

    virtual int32_t val() override {
        return _subscribed->val();
    }

    const std::vector<sp<Integer>>& values() const {
        return _values;
    }

private:
    void updateSubscription() {
        int32_t index = _index->val();
        CHECK(index < _values.size(), "Subscription index out of bounds: index: %d, array_length: %d", index, _values.size());
        _subscribed = _values.at(index % _values.size());
    }

private:
    std::vector<sp<Integer>> _values;
    sp<Integer> _index;
    sp<Integer> _subscribed;
};

}

sp<IntegerWrapper> IntegerType::create(sp<Integer> value)
{
    return sp<IntegerWrapper>::make(std::move(value));
}

sp<IntegerWrapper> IntegerType::create(sp<Numeric> value)
{
    sp<Integer> casted = sp<VariableOP1<int32_t, float>>::make(Operators::Cast<float, int32_t>(), std::move(value));
    return sp<IntegerWrapper>::make(std::move(casted));
}

sp<Integer> IntegerType::create(std::vector<sp<Integer>> values)
{
    return sp<IntegerSubscribed>::make(std::move(values), sp<Integer::Const>::make(0));
}

sp<IntegerWrapper> IntegerType::create(int32_t value)
{
    return sp<IntegerWrapper>::make(value);
}

sp<Integer> IntegerType::add(sp<Integer> self, sp<Integer> rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Add<int32_t>>>::make(std::move(self), std::move(rvalue));
}

sp<Integer> IntegerType::sub(sp<Integer> self, sp<Integer> rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Sub<int32_t>>>::make(std::move(self), std::move(rvalue));
}

sp<Integer> IntegerType::mul(sp<Integer> self, sp<Integer> rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Integer>, Operators::Mul<int32_t>>>::make(std::move(self), std::move(rvalue));
}

sp<Numeric> IntegerType::mul(sp<Integer> self, sp<Numeric> rvalue)
{
    return sp<VariableOP2<sp<Integer>, sp<Numeric>, Operators::Mul<int32_t, float>>>::make(std::move(self), std::move(rvalue));
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

size_t IntegerType::len(const sp<Integer>& self)
{
    sp<IntegerSubscribed> is = self.tryCast<IntegerSubscribed>();
    ASSERT(is);
    return is->values().size();
}

sp<Integer> IntegerType::subscribe(const sp<Integer>& self, sp<Integer> index)
{
    sp<IntegerSubscribed> is = self.tryCast<IntegerSubscribed>();
    ASSERT(is);
    return sp<IntegerSubscribed>::make(is->values(), std::move(index));
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

sp<Integer> IntegerType::delegate(const sp<Integer>& self)
{
    const sp<IntegerWrapper>& iw = self.tryCast<IntegerWrapper>();
    DCHECK_WARN(iw, "Non-IntegerWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return iw ? iw->wrapped() : nullptr;
}

void IntegerType::setDelegate(const sp<Integer>& self, const sp<Integer>& delegate)
{
    const sp<IntegerWrapper> iw = self.tryCast<IntegerWrapper>();
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

sp<Integer> IntegerType::wrap(const sp<Integer>& self)
{
    return sp<IntegerWrapper>::make(self);
}

sp<Integer> IntegerType::freeze(const sp<Integer>& self)
{
    return sp<Integer::Const>::make(self->val());
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
            CHECK(s == "notify", "Unknow Repeat: \"%s, supported enums are [\"none\", \"last\", \"reverse\", \"loop\", \"notify\"]", s.c_str());
            flags |= IntegerType::REPEAT_NOTIFY;
        }
    }
    return static_cast<IntegerType::Repeat>(action | flags);
}

sp<ExpectationI> IntegerType::repeat(std::vector<int32_t> array, IntegerType::Repeat repeat)
{
    const sp<IntegerByArray> delegate = sp<IntegerByArray>::make(sp<IntArray::Vector>::make(std::move(array)), repeat, nullptr);
    return sp<ExpectationI>::make(delegate, delegate.cast<WithObserver>());
}

sp<Integer> IntegerType::animate(const sp<Integer>& self, const sp<Numeric>& interval, const sp<Numeric>& duration)
{
    return sp<Periodic<int32_t>>::make(self, interval ? interval : sp<Numeric>::make<Numeric::Const>(1.0f / 24), duration ? duration : Ark::instance().appClock()->duration());
}

sp<ExpectationI> IntegerType::atLeast(sp<Integer> self, sp<Integer> a1)
{
    const sp<AtLeast<int32_t>> delegate = sp<AtLeast<int32_t>>::make(std::move(self), std::move(a1), nullptr);
    return sp<ExpectationI>::make(std::move(delegate), delegate.cast<WithObserver>());
}

sp<ExpectationI> IntegerType::atMost(sp<Integer> self, sp<Integer> a1)
{
    const sp<AtMost<int32_t>> delegate = sp<AtMost<int32_t>>::make(std::move(self), std::move(a1), nullptr);
    return sp<ExpectationI>::make(delegate, delegate.cast<WithObserver>());
}

sp<ExpectationI> IntegerType::clamp(const sp<Integer>& self, const sp<Integer>& min, const sp<Integer>& max)
{
    DASSERT(self && min && max);
    const sp<Clamp<int32_t>> delegate = sp<Clamp<int32_t>>::make(self, min, max, nullptr);
    return sp<ExpectationI>::make(delegate, delegate.cast<WithObserver>());
}

sp<ExpectationI> IntegerType::fence(const sp<Integer>& self, const sp<Integer>& a1)
{
    DASSERT(self && a1);
    const sp<Fence<int32_t>> delegate = sp<Fence<int32_t>>::make(self, a1, nullptr);
    return sp<ExpectationI>::make(delegate, delegate.cast<WithObserver>());
}

sp<Integer> IntegerType::ifElse(const sp<Integer>& self, const sp<Boolean>& condition, const sp<Integer>& negative)
{
    return sp<VariableTernary<int32_t>>::make(condition, self, negative);
}

sp<Integer> IntegerType::dye(sp<Integer> self, sp<Boolean> condition, String message)
{
    return sp<VariableDyed<int32_t>>::make(std::move(self), std::move(condition), std::move(message));
}

IntegerType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _value(value && value.at(0) == '#' ? sp<Builder<Integer>>::make<Prebuilt>(sp<Integer>::make<NamedType>(value.substr(1))) : Expression::Compiler<int32_t, NumericOperation<int32_t>>().compile(factory, value))
{
    CHECK(_value, "Numeric expression compile failed: %s", value.c_str());
}

sp<Integer> IntegerType::DICTIONARY::build(const Scope& args)
{
    return _value->build(args);
}

IntegerType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory, Documents::ensureAttribute(manifest, constants::VALUE))
{
}

sp<Integer> IntegerType::BUILDER::build(const Scope& args)
{
    return _delegate.build(args);
}

}
