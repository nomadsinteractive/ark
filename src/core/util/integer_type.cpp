#include "core/util/integer_type.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/constants.h"
#include "core/base/expression.h"
#include "core/impl/integer/integer_by_array.h"
#include "core/impl/variable/at_least.h"
#include "core/impl/variable/at_most.h"
#include "core/impl/variable/clamp.h"
#include "core/impl/variable/fence.h"
#include "core/impl/variable/variable_dirty.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_expect.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/inf/array.h"
#include "core/util/operators.h"

namespace ark {

namespace {

class IntegerSubscribed final : public Integer, Implements<IntegerSubscribed, Integer> {
public:
    IntegerSubscribed(Vector<sp<Integer>> values, sp<Integer> index)
        : _values(std::move(values)), _index(std::move(index)) {
        CHECK(!_values.empty(), "IntegerVector should have at least one component");
        updateSubscription();
    }

    bool update(uint64_t timestamp) override {
        const bool indexDirty = _index->update(timestamp);
        if(indexDirty)
            updateSubscription();
        return _subscribed->update(timestamp) || indexDirty;
    }

    int32_t val() override {
        return _subscribed->val();
    }

    const Vector<sp<Integer>>& values() const {
        return _values;
    }

private:
    void updateSubscription() {
        const int32_t index = _index->val();
        CHECK(index < _values.size(), "Subscription index out of bounds: index: %d, array_length: %d", index, _values.size());
        _subscribed = _values.at(index % _values.size());
    }

private:
    Vector<sp<Integer>> _values;
    sp<Integer> _index;
    sp<Integer> _subscribed;
};

}

sp<Integer> IntegerType::create(sp<Integer> value)
{
    return sp<Integer>::make<IntegerWrapper>(std::move(value));
}

sp<Integer> IntegerType::create(sp<Numeric> value)
{
    sp<Integer> casted = sp<VariableOP1<int32_t, float>>::make(Operators::Cast<float, int32_t>(), std::move(value));
    return sp<Integer>::make<IntegerWrapper>(std::move(casted));
}

sp<Integer> IntegerType::create(int32_t value)
{
    return sp<Integer>::make<IntegerWrapper>(value);
}

sp<Integer> IntegerType::add(sp<Integer> lhs, sp<Integer> rhs)
{
    return sp<Integer>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::Add<int32_t>>>(std::move(lhs), std::move(rhs));
}

sp<Integer> IntegerType::sub(sp<Integer> lhs, sp<Integer> rhs)
{
    return sp<Integer>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::Sub<int32_t>>>(std::move(lhs), std::move(rhs));
}

sp<Integer> IntegerType::mul(sp<Integer> lhs, sp<Integer> rhs)
{
    return sp<Integer>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::Mul<int32_t>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> IntegerType::mul(sp<Integer> lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Integer>, sp<Numeric>, Operators::Mul<int32_t, float>>>(std::move(lhs), std::move(rhs));
}

sp<Vec2> IntegerType::mul(sp<Integer> lhs, sp<Vec2> rhs)
{
    return sp<Vec2>::make<VariableOP2<sp<Vec2>, sp<Integer>, Operators::Mul<V2, float>>>(std::move(rhs), std::move(lhs));
}

sp<Vec3> IntegerType::mul(sp<Integer> lhs, sp<Vec3> rhs)
{
    return sp<Vec3>::make<VariableOP2<sp<Vec3>, sp<Integer>, Operators::Mul<V3, float>>>(std::move(rhs), std::move(lhs));
}

sp<Vec4> IntegerType::mul(sp<Integer> lhs, sp<Vec4> rhs)
{
    return sp<Vec4>::make<VariableOP2<sp<Vec4>, sp<Integer>, Operators::Mul<V4, float>>>(std::move(rhs), std::move(lhs));
}

sp<Integer> IntegerType::mod(sp<Integer> lhs, sp<Integer> rhs)
{
    return sp<Integer>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::Mod<int32_t>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> IntegerType::truediv(sp<Integer> lhs, sp<Integer> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::Div<float, int32_t>>>(std::move(lhs), std::move(rhs));
}

sp<Integer> IntegerType::floordiv(sp<Integer> lhs, sp<Integer> rhs)
{
    return sp<Integer>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::FloorDiv<int32_t>>>(std::move(lhs), std::move(rhs));
}

sp<Integer> IntegerType::negative(sp<Integer> self)
{
    return sp<Integer>::make<VariableOP1<int32_t>>(Operators::Neg<int32_t>(), std::move(self));
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
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::GT<int32_t>>>(self, other);
}

sp<Boolean> IntegerType::ge(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::GE<int32_t>>>(self, other);
}

sp<Boolean> IntegerType::lt(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::LT<int32_t>>>(self, other);
}

sp<Boolean> IntegerType::le(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::LE<int32_t>>>(self, other);
}

sp<Boolean> IntegerType::eq(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::EQ<int32_t>>>(self, other);
}

sp<Boolean> IntegerType::ne(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::NE<int32_t>>>(self, other);
}

sp<Boolean> IntegerType::dirty(sp<Integer> self)
{
    return sp<Boolean>::make<VariableDirty<int32_t>>(std::move(self));
}

int32_t IntegerType::val(const sp<Integer>& self)
{
    return self->val();
}

sp<Integer> IntegerType::delegate(const sp<Integer>& self)
{
    const sp<IntegerWrapper>& iw = self.asInstance<IntegerWrapper>();
    DCHECK_WARN(iw, "Non-IntegerWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return iw ? iw->wrapped() : nullptr;
}

void IntegerType::setDelegate(const sp<Integer>& self, const sp<Integer>& delegate)
{
    const sp<IntegerWrapper> iw = self.ensureInstance<IntegerWrapper>("Must be an IntegerWrapper instance to set its delegate attribute");
    iw->set(delegate);
}

sp<Observer> IntegerType::observer(const sp<Integer>& self)
{
    const sp<WithObserver> wo = self.asInstance<WithObserver>();
    return wo ? wo->observer() : nullptr;
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
    return sp<Integer>::make<IntegerWrapper>(self);
}

sp<Integer> IntegerType::freeze(const sp<Integer>& self)
{
    return sp<Integer>::make<Integer::Const>(self->val());
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
    return static_cast<Repeat>(action | flags);
}

sp<Integer> IntegerType::repeat(Vector<int32_t> array, IntegerType::Repeat repeat, sp<Observer> observer)
{
    return sp<Integer>::make<IntegerByArray>(sp<IntArray>::make<IntArray::Vector>(std::move(array)), repeat, std::move(observer));
}

sp<Integer> IntegerType::expect(sp<Integer> self, sp<Boolean> expectation, sp<Observer> observer)
{
    return sp<Integer>::make<VariableExpect<int32_t>>(std::move(self), std::move(expectation), std::move(observer));
}

sp<Integer> IntegerType::atLeast(sp<Integer> self, sp<Integer> a1, sp<Observer> observer)
{
    return sp<Integer>::make<AtLeast<int32_t>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Integer> IntegerType::atMost(sp<Integer> self, sp<Integer> a1, sp<Observer> observer)
{
    return sp<Integer>::make<AtMost<int32_t>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Integer> IntegerType::clamp(sp<Integer> self, sp<Integer> min, sp<Integer> max, sp<Observer> observer)
{
    ASSERT(self && min && max);
    return sp<Integer>::make<Clamp<int32_t>>(std::move(self), std::move(min), std::move(max), std::move(observer));
}

sp<Integer> IntegerType::fence(sp<Integer> self, sp<Integer> a1, sp<Observer> observer)
{
    ASSERT(self && a1);
    return sp<Integer>::make<Fence<int32_t>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Integer> IntegerType::ifElse(const sp<Integer>& self, const sp<Boolean>& condition, const sp<Integer>& negative)
{
    return sp<Integer>::make<VariableTernary<int32_t>>(condition, self, negative);
}

sp<Integer> IntegerType::dye(sp<Integer> self, sp<Boolean> condition, String message)
{
    return sp<Integer>::make<VariableDyed<int32_t>>(std::move(self), std::move(condition), std::move(message));
}

IntegerType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _value(value && value.at(0) == '#' ? sp<Builder<Integer>>::make<Prebuilt>(sp<Integer>::make<Integer::Const>(value.substr(1).hash())) : Expression::Compiler<int32_t, NumericOperation<int32_t>>().compile(factory, value))
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
