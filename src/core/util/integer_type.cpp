#include "core/util/integer_type.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/constants.h"
#include "core/base/expression.h"
#include "core/base/future.h"
#include "core/base/named_hash.h"
#include "core/impl/variable/at_least.h"
#include "core/impl/variable/at_most.h"
#include "core/impl/variable/clamp.h"
#include "core/impl/variable/fence.h"
#include "core/impl/variable/variable_dirty.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/inf/array.h"
#include "core/types/global.h"
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

    bool update(const uint32_t tick) override {
        const bool indexDirty = _index->update(tick);
        if(indexDirty)
            updateSubscription();
        return _subscribed->update(tick) || indexDirty;
    }

    int32_t val() override {
        return _subscribed->val();
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

class IntegerAnimate final : public Integer {
public:
    IntegerAnimate(Vector<HashId> frames, sp<Integer> frameIndex, sp<Future> future)
        : _frames(std::move(frames)), _frame_index(std::move(frameIndex)), _future(std::move(future)), _current_frame_index(0), _last_updated_tick(-1) {
    }

    bool update(const uint32_t tick) override
    {
        const bool dirty = _frame_index->update(tick);
        if(_last_updated_tick != tick)
        {
            _last_updated_tick = tick;
            if(!_future->isDone()->val())
            {
                _current_frame_index = static_cast<uint32_t>(_frame_index->val()) % _frames.size();
                if(_current_frame_index == _frames.size() - 1)
                    _future->notify();
            }
        }
        return dirty;
    }

    int32_t val() override
    {
        return _frames.at(_current_frame_index);
    }

private:
    Vector<HashId> _frames;
    sp<Integer> _frame_index;
    sp<Future> _future;
    uint32_t _current_frame_index;
    uint32_t _last_updated_tick;
};

}

sp<Integer> IntegerType::create(int32_t value)
{
    return sp<Integer>::make<IntegerWrapper>(value);
}

sp<Integer> IntegerType::create(const NamedHash& value)
{
    return sp<Integer>::make<IntegerWrapper>(static_cast<int32_t>(value.hash()));
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

sp<Integer> IntegerType::animate(Vector<HashId> values, sp<Integer> frameIndex, sp<Future> future)
{
    ASSERT(frameIndex && future);
    return sp<Integer>::make<IntegerAnimate>(std::move(values), std::move(frameIndex), std::move(future));
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

sp<Integer> IntegerType::absolute(sp<Integer> self)
{
    return sp<Integer>::make<VariableOP1<int32_t>>(Operators::Abs<int32_t>(), std::move(self));
}

int32_t IntegerType::toInt32(const sp<Integer>& self)
{
    return val(self);
}

int32_t IntegerType::toIndex(const sp<Integer>& self)
{
    return val(self);
}

float IntegerType::toFloat(const sp<Integer>& self)
{
    return static_cast<float>(toInt32(self));
}

sp<Boolean> IntegerType::gt(sp<Integer> self, sp<Integer> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::GT<int32_t>>>(std::move(self), std::move(other));
}

sp<Boolean> IntegerType::ge(sp<Integer> self, sp<Integer> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::GE<int32_t>>>(std::move(self), std::move(other));
}

sp<Boolean> IntegerType::lt(sp<Integer> self, sp<Integer> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::LT<int32_t>>>(std::move(self), std::move(other));
}

sp<Boolean> IntegerType::le(sp<Integer> self, sp<Integer> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::LE<int32_t>>>(std::move(self), std::move(other));
}

sp<Boolean> IntegerType::eq(sp<Integer> self, sp<Integer> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::EQ<int32_t>>>(std::move(self), std::move(other));
}

sp<Boolean> IntegerType::ne(sp<Integer> self, sp<Integer> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Integer>, sp<Integer>, Operators::NE<int32_t>>>(std::move(self), std::move(other));
}

sp<Boolean> IntegerType::dirty(sp<Integer> self)
{
    return sp<Boolean>::make<VariableDirty<int32_t>>(std::move(self));
}

int32_t IntegerType::val(const sp<Integer>& self)
{
    return self->val();
}

void IntegerType::set(const sp<Integer>& self, const int32_t value)
{
    const sp<IntegerWrapper> nw = self.ensureInstance<IntegerWrapper>("Must be an IntegerWrapper instance(result of a wrap() function call) to set its value");
    nw->set(value);
}

void IntegerType::set(const sp<Integer>& self, const NamedHash& value)
{
    const sp<IntegerWrapper> nw = self.ensureInstance<IntegerWrapper>("Must be an IntegerWrapper instance(result of a wrap() function call) to set its value");
    nw->set(value.hash());
}

void IntegerType::set(const sp<Integer>& self, sp<Integer> value)
{
    const sp<IntegerWrapper> nw = self.ensureInstance<IntegerWrapper>("Must be an IntegerWrapper instance(result of a wrap() function call) to set its value");
    nw->set(std::move(value));
}

sp<Integer> IntegerType::wrap(const sp<Integer>& self)
{
    return sp<Integer>::make<IntegerWrapper>(self);
}

int32_t IntegerType::update(const sp<Integer>& self)
{
    self->update(Timestamp::now());
    return self->val();
}

sp<Integer> IntegerType::freeze(const sp<Integer>& self)
{
    return sp<Integer>::make<Integer::Const>(update(self));
}

sp<Integer> IntegerType::atLeast(sp<Integer> self, sp<Integer> a1, sp<Runnable> observer)
{
    return sp<Integer>::make<AtLeast<int32_t>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Integer> IntegerType::atMost(sp<Integer> self, sp<Integer> a1, sp<Runnable> observer)
{
    return sp<Integer>::make<AtMost<int32_t>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Integer> IntegerType::clamp(sp<Integer> self, sp<Integer> min, sp<Integer> max, sp<Runnable> observer)
{
    ASSERT(self && min && max);
    return sp<Integer>::make<Clamp<int32_t>>(std::move(self), std::move(min), std::move(max), std::move(observer));
}

sp<Integer> IntegerType::fence(sp<Integer> self, sp<Integer> a1, sp<Runnable> observer)
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
    CHECK(_value, "Integer expression compile failed: %s. Use \"#%s\" if you need to build a NamedHash integer", value.c_str(), value.c_str());
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
