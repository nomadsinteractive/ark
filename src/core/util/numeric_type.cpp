#include "core/util/numeric_type.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/wrapper.h"
#include "core/base/expression.h"
#include "core/impl/variable/at_least.h"
#include "core/impl/variable/at_most.h"
#include "core/impl/variable/clamp.h"
#include "core/impl/variable/fence.h"
#include "core/impl/variable/integral.h"
#include "core/impl/variable/lerp.h"
#include "core/impl/variable/second_order_dynamics.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/operators.h"

#include "app/base/application_context.h"

namespace ark {

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

sp<Numeric> NumericType::add(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Add<float>>>(lvalue, rvalue);
}

sp<Numeric> NumericType::add(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, float, Operators::Add<float>>>(lvalue, rvalue);
}

sp<Numeric> NumericType::add(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<Numeric>::make<VariableOP2<float, sp<Numeric>, Operators::Add<float>>>(lvalue, rvalue);
}

sp<Numeric> NumericType::sub(sp<Numeric> lvalue, sp<Numeric> rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Sub<float>>>(std::move(lvalue), std::move(rvalue));
}

sp<Numeric> NumericType::sub(sp<Numeric> lvalue, float rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, float, Operators::Sub<float>>>(std::move(lvalue), rvalue);
}

sp<Numeric> NumericType::sub(float lvalue, sp<Numeric> rvalue)
{
    return sp<Numeric>::make<VariableOP2<float, sp<Numeric>, Operators::Sub<float>>>(lvalue, std::move(rvalue));
}

sp<Numeric> NumericType::mul(sp<Numeric> lvalue, sp<Numeric> rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::Mul<float>>>(std::move(lvalue), std::move(rvalue));
}

sp<Numeric> NumericType::mul(sp<Numeric> lvalue, float rvalue)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, float, Operators::Mul<float>>>(std::move(lvalue), rvalue);
}

sp<Vec2> NumericType::mul(sp<Numeric> lvalue, sp<Vec2> rvalue)
{
    return sp<Vec2>::make<VariableOP2<sp<Vec2>, sp<Numeric>, Operators::Mul<V2, float>>>(std::move(rvalue), std::move(lvalue));
}

sp<Vec3> NumericType::mul(sp<Numeric> lvalue, sp<Vec3> rvalue)
{
    return sp<Vec3>::make<VariableOP2<sp<Vec3>, sp<Numeric>, Operators::Mul<V3, float>>>(std::move(rvalue), std::move(lvalue));
}

sp<Vec4> NumericType::mul(sp<Numeric> lvalue, sp<Vec4> rvalue)
{
    return sp<Vec4>::make<VariableOP2<sp<Vec4>, sp<Numeric>, Operators::Mul<V4, float>>>(std::move(rvalue), std::move(lvalue));
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

sp<Numeric> NumericType::negative(const sp<Numeric>& self)
{
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Neg<float>(), self);
}

sp<Numeric> NumericType::absolute(const sp<Numeric>& self)
{
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Abs<float>(), self);
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
    return static_cast<int32_t>(self->val());
}

float NumericType::toFloat(const sp<Numeric>& self)
{
    return self->val();
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

float NumericType::val(const sp<Numeric>& self)
{
    return self->val();
}

sp<Numeric> NumericType::delegate(const sp<Numeric>& self)
{
    const sp<NumericWrapper>& nw = self.asInstance<NumericWrapper>();
    DCHECK_WARN(nw, "Non-NumericWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return nw ? nw->wrapped() : nullptr;
}

void NumericType::setDelegate(const sp<Numeric>& self, const sp<Numeric>& delegate)
{
    const sp<NumericWrapper> nw = self.ensureInstance<NumericWrapper>("Must be an NumericWrapper instance to set its delegate attribute");
    nw->set(delegate);
}

sp<Observer> NumericType::observer(const sp<Numeric>& self)
{
    const sp<WithObserver> wo = self.asInstance<WithObserver>();
    return wo ? wo->observer() : nullptr;
}

void NumericType::set(const sp<Numeric::Impl>& self, float value)
{
    self->set(value);
}

void NumericType::set(const sp<NumericWrapper>& self, float value)
{
    self->set(value);
}

void NumericType::set(const sp<NumericWrapper>& self, sp<Numeric> value)
{
    self->set(std::move(value));
}

float NumericType::fix(const sp<Numeric>& self)
{
    const sp<NumericWrapper> iw = self.asInstance<NumericWrapper>();
    DCHECK_WARN(iw, "Calling fix on non-NumericWrapper has no effect.");
    if(iw)
        return iw->fix();
    return 0;
}

sp<Numeric> NumericType::freeze(const sp<Numeric>& self)
{
    return sp<Numeric>::make<NumericWrapper>(self->val());
}

sp<Numeric> NumericType::wrap(sp<Numeric> self)
{
    return sp<Numeric>::make<NumericWrapper>(std::move(self));
}

sp<Numeric> NumericType::synchronize(sp<Numeric> self, sp<Boolean> discarded)
{
    return Ark::instance().applicationContext()->synchronize(std::move(self), std::move(discarded));
}

sp<Numeric> NumericType::atLeast(sp<Numeric> self, sp<Numeric> a1, sp<Observer> observer)
{
    return sp<Numeric>::make<AtLeast<float>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Numeric> NumericType::atMost(sp<Numeric> self, sp<Numeric> a1, sp<Observer> observer)
{
    return sp<Numeric>::make<AtMost<float>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Numeric> NumericType::clamp(sp<Numeric> self, sp<Numeric> min, sp<Numeric> max, sp<Observer> observer)
{
    ASSERT(self && min && max);
    return sp<Numeric>::make<Clamp<float>>(std::move(self), std::move(min), std::move(max), std::move(observer));
}

sp<Numeric> NumericType::fence(sp<Numeric> self, sp<Numeric> a1, sp<Observer> observer)
{
    DASSERT(self && a1);
    return sp<Numeric>::make<Fence<float>>(std::move(self), std::move(a1), std::move(observer));
}

sp<Numeric> NumericType::ifElse(sp<Numeric> self, sp<Boolean> condition, sp<Numeric> negative)
{
    return sp<Numeric>::make<VariableTernary<float>>(std::move(condition), std::move(self), std::move(negative));
}

sp<Numeric> NumericType::dye(sp<Numeric> self, sp<Boolean> condition, String message)
{
    return sp<Numeric>::make<VariableDyed<float>>(std::move(self), std::move(condition), std::move(message));
}

sp<Numeric> NumericType::lerp(const sp<Numeric>& self, const sp<Numeric>& b, const sp<Numeric>& t)
{
    return sp<Numeric>::make<Lerp<float, float>>(self, b, t);
}

sp<Numeric> NumericType::sod(sp<Numeric> self, float d0, float k, float z, float r, sp<Numeric> t)
{
    if(!t)
        t = Ark::instance().appClock()->duration();
    return sp<Numeric>::make<SecondOrderDynamics<float>>(std::move(self), d0, std::move(t), k, z, r);
}

sp<Numeric> NumericType::modFloor(const sp<Numeric>& self, const sp<Numeric>& mod)
{
    return sp<Numeric>::make<VariableOP2<sp<Numeric>, sp<Numeric>, Operators::ModFloor<float>>>(self, mod);
}

sp<Numeric> NumericType::floor(sp<Numeric> self)
{
    return sp<Numeric>::make<VariableOP1<float>>(Operators::Floor<float>(), std::move(self));
}

sp<Numeric> NumericType::ceil(sp<Numeric> self)
{
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
