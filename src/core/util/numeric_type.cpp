#include "core/util/numeric_type.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/wrapper.h"
#include "core/base/expression.h"
#include "core/impl/numeric/approach.h"
#include "core/impl/numeric/stalker.h"
#include "core/impl/numeric/vibrate.h"
#include "core/impl/variable/at_least.h"
#include "core/impl/variable/at_most.h"
#include "core/impl/variable/clamp.h"
#include "core/impl/variable/fence.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/integral_with_resistance.h"
#include "core/impl/variable/interpolate.h"
#include "core/impl/variable/periodic.h"
#include "core/impl/variable/second_order_dynamics.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/operators.h"
#include "core/util/strings.h"
#include "core/util/variable_util.h"

namespace ark {

sp<Numeric> NumericType::create(const sp<Numeric>& value)
{
    return sp<NumericWrapper>::make(value);
}

sp<Numeric> NumericType::create(float value)
{
    return sp<NumericWrapper>::make(value);
}

sp<Numeric> NumericType::add(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Add<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::add(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Numeric>, float, Operators::Add<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::add(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, sp<Numeric>, Operators::Add<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::sub(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Sub<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::sub(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Numeric>, float, Operators::Sub<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::sub(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, sp<Numeric>, Operators::Sub<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::mul(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Mul<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::mul(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Numeric>, float, Operators::Mul<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::mul(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, sp<Numeric>, Operators::Mul<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::truediv(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Div<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::truediv(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Numeric>, float, Operators::Div<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::truediv(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, sp<Numeric>, Operators::Div<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::floordiv(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::NumericFloorDiv<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::floordiv(const sp<Numeric>& lvalue, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<sp<Numeric>, sp<Integer>, Operators::NumericFloorDiv<float, int32_t>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::mod(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Mod<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::mod(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<sp<Numeric>, float, Operators::Mod<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::mod(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, sp<Numeric>, Operators::Mod<float>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericType::negative(const sp<Numeric>& self)
{
    return sp<VariableOP1<float>>::make(Operators::Neg<float>(), self);
}

sp<Numeric> NumericType::absolute(const sp<Numeric>& self)
{
    return sp<VariableOP1<float>>::make(Operators::Abs<float>(), self);
}

sp<Numeric> NumericType::pow(const sp<Numeric>& x, const sp<Integer>& y, const sp<Integer>& /*z*/)
{
    return sp<VariableOP2<sp<Numeric>, sp<Integer>, Operators::Pow<float, int32_t>>>::make(x, y);
}

sp<Numeric> NumericType::pow(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Integer>& /*z*/)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Pow<float>>>::make(x, y);
}

int32_t NumericType::toInt32(const sp<Numeric>& self)
{
    return static_cast<int32_t>(self->val());
}

float NumericType::toFloat(const sp<Numeric>& self)
{
    return self->val();
}

sp<Integer> NumericType::toInteger(sp<Numeric> self)
{
    return sp<VariableOP1<int32_t, float>>::make(Operators::Cast<float, int32_t>(), std::move(self));
}

sp<Boolean> NumericType::gt(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::GT<float>>>::make(self, other);
}

sp<Boolean> NumericType::ge(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::GE<float>>>::make(self, other);
}

sp<Boolean> NumericType::lt(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::LT<float>>>::make(self, other);
}

sp<Boolean> NumericType::le(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::LE<float>>>::make(self, other);
}

sp<Boolean> NumericType::eq(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::EQ<float>>>::make(self, other);
}

sp<Boolean> NumericType::ne(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::NE<float>>>::make(self, other);
}

float NumericType::val(const sp<Numeric>& self)
{
    return self->val();
}

const sp<Numeric>& NumericType::delegate(const sp<Numeric>& self)
{
    const sp<NumericWrapper> nw = self.as<NumericWrapper>();
    DWARN(nw, "Non-NumericWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return nw ? nw->wrapped() : sp<Numeric>::null();
}

void NumericType::setDelegate(const sp<Numeric>& self, const sp<Numeric>& delegate)
{
    const sp<NumericWrapper> nw = self.as<NumericWrapper>();
    CHECK(nw, "Must be an NumericWrapper instance to set its delegate attribute");
    nw->set(delegate);
}

void NumericType::set(const sp<Numeric::Impl>& self, float value)
{
    self->set(value);
}

void NumericType::set(const sp<NumericWrapper>& self, float value)
{
    self->set(value);
}

void NumericType::set(const sp<NumericWrapper>& self, const sp<Numeric>& delegate)
{
    self->set(delegate);
}

float NumericType::fix(const sp<Numeric>& self)
{
    const sp<NumericWrapper> iw = self.as<NumericWrapper>();
    DWARN(iw, "Calling fix on non-NumericWrapper has no effect.");
    if(iw)
        return iw->fix();
    return 0;
}

sp<Numeric> NumericType::freeze(const sp<Numeric>& self)
{
    return sp<NumericWrapper>::make(self->val());
}

sp<Numeric> NumericType::wrap(const sp<Numeric>& self)
{
    return sp<NumericWrapper>::make(self);
}

sp<Numeric> NumericType::synchronize(const sp<Numeric>& self, const sp<Boolean>& disposed)
{
    return Ark::instance().applicationContext()->synchronize(self, disposed);
}

sp<ExpectationF> NumericType::approach(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    Notifier notifier;
    return sp<ExpectationF>::make(sp<Approach>::make(self, a1, notifier), std::move(notifier));
}

sp<ExpectationF> NumericType::atLeast(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    Notifier notifier;
    sp<Numeric> delegate = sp<AtLeast<float>>::make(self, a1, notifier);
    return sp<ExpectationF>::make(std::move(delegate), std::move(notifier));
}

sp<ExpectationF> NumericType::atMost(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    Notifier notifier;
    sp<Numeric> delegate = sp<AtMost<float>>::make(self, a1, notifier);
    return sp<ExpectationF>::make(std::move(delegate), std::move(notifier));
}

sp<ExpectationF> NumericType::boundary(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    DASSERT(self && a1);
    return self->val() < a1->val() ? atMost(self, a1) : atLeast(self, a1);
}

sp<ExpectationF> NumericType::clamp(const sp<Numeric>& self, const sp<Numeric>& min, const sp<Numeric>& max)
{
    DASSERT(self && min && max);
    Notifier notifier;
    sp<Numeric> delegate = sp<Clamp<float>>::make(self, min, max, notifier);
    return sp<ExpectationF>::make(std::move(delegate), std::move(notifier));
}

sp<ExpectationF> NumericType::fence(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    DASSERT(self && a1);
    Notifier notifier;
    sp<Numeric> delegate = sp<Fence<float>>::make(self, a1, notifier);
    return sp<ExpectationF>::make(std::move(delegate), std::move(notifier));
}

sp<Numeric> NumericType::ifElse(const sp<Numeric>& self, const sp<Boolean>& condition, const sp<Numeric>& negative)
{
    return sp<VariableTernary<float>>::make(condition, self, negative);
}

sp<Numeric> NumericType::pursue(float s0, const sp<Numeric>& target, float duration, const sp<Numeric>& t)
{
    return sp<Stalker>::make(t ? t : Ark::instance().appClock()->duration(), target, s0, duration);
}

sp<Numeric> NumericType::vibrate(float s0, float v0, float s1, float v1, float duration, const sp<Numeric>& t)
{
    DCHECK(duration > 0, "Duration must be greater than zero");
    float o, a, t0, t1;
    Math::vibrate(s0, v0, s1, v1, o, a, t0, t1);
    float multiplier = (t1 - t0) / duration;
    const sp<Numeric> b = sp<Numeric::Const>::make(t1 - t0);
    return sp<Vibrate>::make(boundary(mul(t ? t : Ark::instance().appClock()->duration(), multiplier), b)->wrapped(), a, t0, o);
}

sp<Numeric> NumericType::lerp(const sp<Numeric>& self, const sp<Numeric>& b, const sp<Numeric>& t)
{
    return sp<Interpolate<float, float>>::make(self, b, t);
}

sp<Numeric> NumericType::sod(sp<Numeric> self, float k, float z, float r, sp<Numeric> t)
{
    if(t == nullptr)
        t = Ark::instance().appClock()->duration();
    return sp<SecondOrderDynamics<float>>::make(std::move(self), std::move(t), k, z, r);
}

sp<Numeric> NumericType::periodic(const sp<Numeric>& self, const sp<Numeric>& interval, const sp<Numeric>& duration)
{
    return sp<Periodic<float>>::make(self, interval ? interval : sp<Numeric>::make<Numeric::Const>(1.0f / 24), duration ? duration : Ark::instance().appClock()->duration());
}

sp<Numeric> NumericType::integralWithResistance(const sp<Numeric>& self, float v0, const sp<Numeric>& cd, const sp<Numeric>& t)
{
    return sp<IntegralWithResistance<float>>::make(v0, self, cd, t ? t : Ark::instance().appClock()->duration());
}

sp<Numeric> NumericType::modFloor(const sp<Numeric>& self, const sp<Numeric>& mod)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::ModFloor<float>>>::make(self, mod);
}

sp<Numeric> NumericType::modCeil(const sp<Numeric>& self, const sp<Numeric>& mod)
{
    return sp<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::ModCeil<float>>>::make(self, mod);
}

sp<Numeric> NumericType::attract(const sp<Numeric>& self, float s0, float duration, const sp<Numeric>& t)
{
    return sp<Stalker>::make(t ? t : Ark::instance().appClock()->duration(), self, s0, duration);
}

sp<Numeric> NumericType::integral(const sp<Numeric>& self, const sp<Numeric>& t)
{
    return sp<Integral<float>>::make(self, t ? t : Ark::instance().appClock()->duration());
}

NumericType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(Expression::Compiler<float, NumericOperation<float>>().compile(factory, expr.strip()))
{
    DCHECK(_value, "Numeric expression compile failed: %s", expr.c_str());
}

sp<Numeric> NumericType::DICTIONARY::build(const Scope& args)
{
    return _value->build(args);
}

NumericType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(Expression::Compiler<float, NumericOperation<float>>().compile(factory, Documents::ensureAttribute(manifest, Constants::Attributes::VALUE)))
{
}

sp<Numeric> NumericType::BUILDER::build(const Scope& args)
{
    return _value->build(args);
}

}
