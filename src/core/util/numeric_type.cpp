#include "core/util/numeric_type.h"

#include "core/ark.h"
#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/wrapper.h"
#include "core/impl/variable/at_least.h"
#include "core/impl/variable/at_most.h"
#include "core/impl/variable/clamp.h"
#include "core/impl/variable/fence.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/lerp.h"
#include "core/impl/variable/second_order_dynamics.h"
#include "core/impl/variable/variable_dirty.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/impl/variable/variable_tracking.h"
#include "core/util/boolean_type.h"
#include "core/util/expression.h"
#include "core/util/operators.h"

#include "app/base/application_context.h"

namespace ark {

namespace {

class BooleanAlmostEqual final : public Boolean {
public:
    BooleanAlmostEqual(sp<Numeric> a1, sp<Numeric> a2, const float tolerance)
        : _a1(std::move(a1)), _a2(std::move(a2)), _tolerance(std::max(tolerance, std::numeric_limits<float>::min()))
    {
    }

    bool update(uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _a1, _a2);
    }

    bool val() override
    {
        return std::abs(_a1->val() - _a2->val()) <= _tolerance;
    }

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
    float _tolerance;
};

}


sp<NumericWrapper> NumericType::create(float value)
{
    return sp<NumericWrapper>::make(value);
}

sp<NumericWrapper> NumericType::create(sp<Numeric> value)
{
    return sp<NumericWrapper>::make(std::move(value));
}

sp<NumericWrapper> NumericType::create(sp<Integer> value)
{
    sp<Numeric> casted = sp<VariableOP1<float, int32_t>>::make(Operators::Cast<int32_t, float>(), std::move(value));
    return sp<NumericWrapper>::make(std::move(casted));
}

sp<Numeric> NumericType::add(sp<Numeric> lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Add<float>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> NumericType::add(sp<Numeric> lhs, float rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, float, Operators::Add<float>>>(std::move(lhs), rhs);
}

sp<Numeric> NumericType::add(float lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<float, sp<Numeric>, Operators::Add<float>>>(lhs, std::move(rhs));
}

sp<Numeric> NumericType::sub(sp<Numeric> lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Sub<float>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> NumericType::sub(sp<Numeric> lhs, float rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, float, Operators::Sub<float>>>(std::move(lhs), rhs);
}

sp<Numeric> NumericType::sub(float lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<float, sp<Numeric>, Operators::Sub<float>>>(lhs, std::move(rhs));
}

sp<Numeric> NumericType::mul(sp<Numeric> lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Mul<float>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> NumericType::mul(sp<Numeric> lhs, float rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, float, Operators::Mul<float>>>(std::move(lhs), rhs);
}

sp<Vec2> NumericType::mul(sp<Numeric> lhs, sp<Vec2> rhs)
{
    return sp<Vec2>::make<VariableOP2<sp<Vec2>, sp<Numeric>, Operators::Mul<V2, float>>>(std::move(rhs), std::move(lhs));
}

sp<Vec3> NumericType::mul(sp<Numeric> lhs, sp<Vec3> rhs)
{
    return sp<Vec3>::make<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Mul<V3, float>>>(std::move(rhs), std::move(lhs));
}

sp<Vec4> NumericType::mul(sp<Numeric> lhs, sp<Vec4> rhs)
{
    return sp<Vec4>::make<VariableOP2<sp<Vec4>, sp<Numeric>, Operators::Mul<V4, float>>>(std::move(rhs), std::move(lhs));
}

sp<Numeric> NumericType::truediv(sp<Numeric> lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Div<float>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> NumericType::floordiv(sp<Numeric> lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::FloorDiv<float>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> NumericType::floordiv(sp<Numeric> lhs, sp<Integer> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Integer>, Operators::FloorDiv<float, int32_t>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> NumericType::mod(sp<Numeric> lhs, sp<Numeric> rhs)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Mod<float>>>(std::move(lhs), std::move(rhs));
}

sp<Numeric> NumericType::negative(sp<Numeric> self)
{
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Neg<float>(), std::move(self));
}

sp<Numeric> NumericType::absolute(sp<Numeric> self)
{
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Abs<float>(), std::move(self));
}

sp<Numeric> NumericType::pow(const sp<Numeric>& x, const sp<Integer>& y, const sp<Integer>& /*z*/)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Integer>, Operators::Pow<float, int32_t>>>(x, y);
}

sp<Numeric> NumericType::pow(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Integer>& /*z*/)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Pow<float>>>(x, y);
}

int32_t NumericType::toInt32(const sp<Numeric>& self)
{
    return static_cast<int32_t>(update(self));
}

float NumericType::toFloat(const sp<Numeric>& self)
{
    return update(self);
}

sp<Boolean> NumericType::gt(sp<Numeric> self, sp<Numeric> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::GT<float>>>(std::move(self), std::move(other));
}

sp<Boolean> NumericType::ge(sp<Numeric> self, sp<Numeric> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::GE<float>>>(std::move(self), std::move(other));
}

sp<Boolean> NumericType::lt(sp<Numeric> self, sp<Numeric> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::LT<float>>>(std::move(self), std::move(other));
}

sp<Boolean> NumericType::le(sp<Numeric> self, sp<Numeric> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::LE<float>>>(std::move(self), std::move(other));
}

sp<Boolean> NumericType::eq(sp<Numeric> self, sp<Numeric> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::EQ<float>>>(std::move(self), std::move(other));
}

sp<Boolean> NumericType::ne(sp<Numeric> self, sp<Numeric> other)
{
    return sp<Boolean>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::NE<float>>>(std::move(self), std::move(other));
}

sp<Boolean> NumericType::dirty(sp<Numeric> self)
{
    return sp<Boolean>::make<VariableDirty<float>>(std::move(self));
}

float NumericType::val(const sp<Numeric>& self)
{
    return self->val();
}

void NumericType::set(const sp<Numeric::Impl>& self, const float value)
{
    self->set(value);
}

void NumericType::set(const sp<NumericWrapper>& self, const float value)
{
    self->set(value);
}

void NumericType::set(const sp<NumericWrapper>& self, sp<Numeric> value)
{
    self->set(std::move(value));
}

float NumericType::update(const sp<Numeric>& self)
{
    self->update(Timestamp::now());
    return self->val();
}

sp<Numeric> NumericType::freeze(const sp<Numeric>& self)
{
    return sp<Numeric>::make<NumericWrapper>(update(self));
}

sp<Numeric> NumericType::wrap(sp<Numeric> self)
{
    return sp<Numeric>::make<NumericWrapper>(std::move(self));
}

sp<Numeric> NumericType::synchronize(sp<Numeric> self, sp<Boolean> canceled)
{
    return Ark::instance().renderController()->synchronize(std::move(self), std::move(canceled));
}

sp<Numeric> NumericType::atLeast(sp<Numeric> self, sp<Numeric> a1, sp<Runnable> callback)
{
    return sp<Numeric>::make<AtLeast<float>>(std::move(self), std::move(a1), std::move(callback));
}

sp<Numeric> NumericType::atMost(sp<Numeric> self, sp<Numeric> a1, sp<Runnable> callback)
{
    return sp<Numeric>::make<AtMost<float>>(std::move(self), std::move(a1), std::move(callback));
}

sp<Numeric> NumericType::clamp(sp<Numeric> self, sp<Numeric> min, sp<Numeric> max, sp<Runnable> callback)
{
    ASSERT(self && min && max);
    return sp<Numeric>::make<Clamp<float>>(std::move(self), std::move(min), std::move(max), std::move(callback));
}

sp<Numeric> NumericType::fence(sp<Numeric> self, sp<Numeric> a1, sp<Runnable> callback)
{
    DASSERT(self && a1);
    return sp<Numeric>::make<Fence<float>>(std::move(self), std::move(a1), std::move(callback));
}

sp<Numeric> NumericType::ifElse(sp<Numeric> self, sp<Boolean> condition, sp<Numeric> negative)
{
    return sp<Numeric>::make<VariableTernary<float>>(std::move(condition), std::move(self), std::move(negative));
}

sp<Numeric> NumericType::dye(sp<Numeric> self, sp<Boolean> condition, String message)
{
    return sp<Numeric>::make<VariableDyed<float>>(std::move(self), std::move(condition), std::move(message));
}

String NumericType::str(const sp<Numeric>& self)
{
    return Strings::sprintf("%.2f", update(self));
}

sp<Numeric> NumericType::lerp(sp<Numeric> self, sp<Numeric> b, sp<Numeric> t)
{
    return sp<Numeric>::make<Lerp<float, float>>(std::move(self), std::move(b), std::move(t));
}

sp<Numeric> NumericType::track(sp<Numeric> self, float s0, float speed, float distance, sp<Future> future, sp<Numeric> t)
{
    if(!t)
        t = Ark::instance().appClock()->duration();
    return sp<Numeric>::make<VariableTracking<float>>(std::move(self), std::move(t), s0, speed, distance, std::move(future));
}

sp<Numeric> NumericType::sod(sp<Numeric> self, const float s0, const float f, const float z, const float r, sp<Numeric> t)
{
    if(!t)
        t = Ark::instance().appClock()->duration();
    return sp<Numeric>::make<SecondOrderDynamics<float>>(std::move(self), s0, std::move(t), f, z, r);
}

sp<Numeric> NumericType::floor(sp<Numeric> self, sp<Numeric> mod)
{
    if(mod)
        return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::ModFloor<float>>>(std::move(self), std::move(mod));
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Floor<float>(), std::move(self));
}

sp<Numeric> NumericType::ceil(sp<Numeric> self, sp<Numeric> mod)
{
    if(mod)
        return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::ModCeil<float>>>(std::move(self), std::move(mod));
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Ceil<float>(), std::move(self));
}

sp<Numeric> NumericType::round(sp<Numeric> self)
{
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Round<float>(), std::move(self));
}

sp<Numeric> NumericType::integral(const sp<Numeric>& self, const sp<Numeric>& t)
{
    return sp<Numeric>::make<Integral<float>>(self, t ? t : Ark::instance().appClock()->duration());
}

sp<Boolean> NumericType::almostEqual(sp<Numeric> self, sp<Numeric> other, float tolerance)
{
    return sp<Boolean>::make<BooleanAlmostEqual>(std::move(self), std::move(other), tolerance);
}

sp<Numeric> NumericType::normalize(sp<Numeric> self)
{
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Normalize<float>(), std::move(self));
}

sp<Numeric> NumericType::distance(sp<Numeric> self, sp<Numeric> other)
{
    return absolute(sub(std::move(self), std::move(other)));
}

sp<Numeric> NumericType::distance2(sp<Numeric> self, sp<Numeric> other)
{
    const sp<Numeric> d1 = sub(std::move(self), std::move(other));
    return mul(d1, d1);
}

NumericType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(Expression::Compiler<float, NumericOperation<float>>().compile(factory, expr.strip()))
{
    CHECK(_value, "Numeric expression compile failed: %s", expr.c_str());
}

sp<Numeric> NumericType::DICTIONARY::build(const Scope& args)
{
    return _value->build(args);
}

NumericType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(Expression::Compiler<float, NumericOperation<float>>().compile(factory, Documents::ensureAttribute(manifest, constants::VALUE)))
{
}

sp<Numeric> NumericType::BUILDER::build(const Scope& args)
{
    return _value->build(args);
}

}
