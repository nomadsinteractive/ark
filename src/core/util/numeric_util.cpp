#include "core/util/numeric_util.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/delegate.h"
#include "core/base/expectation.h"
#include "core/base/expression.h"
#include "core/impl/builder/builder_by_instance.h"
#include "core/impl/numeric/approach.h"
#include "core/impl/numeric/clamp.h"
#include "core/impl/numeric/fence.h"
#include "core/impl/numeric/integral.h"
#include "core/impl/numeric/max.h"
#include "core/impl/numeric/min.h"
#include "core/impl/numeric/numeric_negative.h"
#include "core/impl/numeric/stalker.h"
#include "core/impl/numeric/vibrate.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/operators.h"
#include "core/util/strings.h"

namespace ark {

namespace {

class AtLeast : public Numeric, public Delegate<Numeric>, Implements<AtLeast, Numeric, Delegate<Numeric>> {
public:
    AtLeast(const sp<Numeric>& delegate, const sp<Numeric>& boundary, Notifier notifier)
         : Delegate<Numeric>(delegate), _boundary(boundary), _notifer(std::move(notifier)) {
    }

    virtual float val() override {
        float value = _delegate->val();
        float boundary = _boundary->val();
        if(value < boundary) {
            _notifer.notify();
            return boundary;
        }
        return value;
    }

private:
    sp<Numeric> _boundary;

    Notifier _notifer;
};


class AtMost : public Numeric, public Delegate<Numeric>, Implements<AtMost, Numeric, Delegate<Numeric>> {
public:
    AtMost(const sp<Numeric>& delegate, const sp<Numeric>& boundary, Notifier notifier)
         : Delegate<Numeric>(delegate), _boundary(boundary), _notifer(std::move(notifier)) {
    }

    virtual float val() override {
        float value = _delegate->val();
        float boundary = _boundary->val();
        if(value > boundary) {
            _notifer.notify();
            return boundary;
        }
        return value;
    }

private:
    sp<Numeric> _boundary;

    Notifier _notifer;
};

}


sp<Numeric> NumericUtil::create(const sp<Numeric>& value)
{
    return sp<NumericWrapper>::make(value);
}

sp<Numeric> NumericUtil::create(float value)
{
    return sp<NumericWrapper>::make(value);
}

sp<Numeric> NumericUtil::add(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Add<float>, sp<Numeric>, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::add(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<float, float, Operators::Add<float>, sp<Numeric>, float>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::add(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Add<float>, float, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::sub(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Sub<float>, sp<Numeric>, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::sub(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<float, float, Operators::Sub<float>, sp<Numeric>, float>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::sub(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Sub<float>, float, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::mul(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Mul<float>, sp<Numeric>, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::mul(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<float, float, Operators::Mul<float>, sp<Numeric>, float>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::mul(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Mul<float>, float, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::truediv(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Div<float>, sp<Numeric>, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::truediv(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<float, float, Operators::Div<float>, sp<Numeric>, float>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::truediv(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Div<float>, float, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::floordiv(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Numeric> NumericUtil::mod(const sp<Numeric>& lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Mod<float>, sp<Numeric>, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::mod(const sp<Numeric>& lvalue, float rvalue)
{
    return sp<VariableOP2<float, float, Operators::Mod<float>, sp<Numeric>, float>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::mod(float lvalue, const sp<Numeric>& rvalue)
{
    return sp<VariableOP2<float, float, Operators::Mod<float>, float, sp<Numeric>>>::make(lvalue, rvalue);
}

sp<Numeric> NumericUtil::negative(const sp<Numeric>& self)
{
    return sp<NumericNegative>::make(self);
}

sp<Numeric> NumericUtil::pow(const sp<Numeric>& x, const sp<Integer>& y, const sp<Integer>& /*z*/)
{
    return sp<VariableOP2<float, int32_t, Operators::Pow<float, int32_t>, sp<Numeric>, sp<Integer>>>::make(x, y);
}

sp<Numeric> NumericUtil::pow(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Integer>& /*z*/)
{
    return sp<VariableOP2<float, float, Operators::Pow<float>, sp<Numeric>, sp<Numeric>>>::make(x, y);
}

int32_t NumericUtil::toInt32(const sp<Numeric>& self)
{
    return static_cast<int32_t>(self->val());
}

float NumericUtil::toFloat(const sp<Numeric>& self)
{
    return self->val();
}

sp<Boolean> NumericUtil::gt(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<float, float, Operators::GT<float>, sp<Numeric>, sp<Numeric>>>::make(self, other);
}

sp<Boolean> NumericUtil::ge(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<float, float, Operators::GE<float>, sp<Numeric>, sp<Numeric>>>::make(self, other);
}

sp<Boolean> NumericUtil::lt(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<float, float, Operators::LT<float>, sp<Numeric>, sp<Numeric>>>::make(self, other);
}

sp<Boolean> NumericUtil::le(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<float, float, Operators::LE<float>, sp<Numeric>, sp<Numeric>>>::make(self, other);
}

sp<Boolean> NumericUtil::eq(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<float, float, Operators::EQ<float>, sp<Numeric>, sp<Numeric>>>::make(self, other);
}

sp<Boolean> NumericUtil::ne(const sp<Numeric>& self, const sp<Numeric>& other)
{
    return sp<VariableOP2<float, float, Operators::NE<float>, sp<Numeric>, sp<Numeric>>>::make(self, other);
}

float NumericUtil::val(const sp<Numeric>& self)
{
    return self->val();
}

void NumericUtil::setVal(const sp<Numeric>& self, float value)
{
    const sp<NumericWrapper> nw = self.as<NumericWrapper>();
    if(nw)
        nw->set(value);
    else
    {
        const sp<Numeric::Impl> ni = self.as<Numeric::Impl>();
        DCHECK(ni, "Numeric instance should be either NumericWrapper or Numeric::Impl to be set its value.");
        ni->set(value);
    }
}

const sp<Numeric>& NumericUtil::delegate(const sp<Numeric>& self)
{
    const sp<NumericWrapper> nw = self.as<NumericWrapper>();
    DWARN(nw, "Non-NumericWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return nw ? nw->delegate() : sp<Numeric>::null();
}

void NumericUtil::setDelegate(const sp<Numeric>& self, const sp<Numeric>& delegate)
{
    const sp<NumericWrapper> nw = self.as<NumericWrapper>();
    DCHECK(nw, "Must be an NumericWrapper instance to set its delegate attribute");
    nw->set(delegate);
}

void NumericUtil::set(const sp<Numeric::Impl>& self, float value)
{
    self->set(value);
}

void NumericUtil::set(const sp<NumericWrapper>& self, float value)
{
    self->set(value);
}

void NumericUtil::set(const sp<NumericWrapper>& self, const sp<Numeric>& delegate)
{
    self->set(delegate);
}

float NumericUtil::fix(const sp<Numeric>& self)
{
    const sp<NumericWrapper> iw = self.as<NumericWrapper>();
    DWARN(iw, "Calling fix on non-NumericWrapper has no effect.");
    if(iw)
        return iw->fix();
    return 0;
}

sp<Expectation> NumericUtil::approach(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    Notifier notifier;
    return sp<Expectation>::make(sp<Approach>::make(self, a1, notifier), std::move(notifier));
}

sp<Expectation> NumericUtil::atLeast(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    DASSERT(self && a1);
    Notifier notifier;
    return sp<Expectation>::make(sp<AtLeast>::make(self, a1, notifier), std::move(notifier));
}

sp<Expectation> NumericUtil::atMost(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    DASSERT(self && a1);
    Notifier notifier;
    return sp<Expectation>::make(sp<AtMost>::make(self, a1, notifier), std::move(notifier));
}

sp<Expectation> NumericUtil::boundary(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    DASSERT(self && a1);
    return self->val() < a1->val() ? atMost(self, a1) : atLeast(self, a1);
}

sp<Expectation> NumericUtil::clamp(const sp<Numeric>& self, const sp<Numeric>& min, const sp<Numeric>& max)
{
    DASSERT(self && min && max);
    Notifier notifier;
    return sp<Expectation>::make(sp<Clamp>::make(self, min, max, notifier), std::move(notifier));
}

sp<Expectation> NumericUtil::fence(const sp<Numeric>& self, const sp<Numeric>& a1)
{
    DASSERT(self && a1);
    Notifier notifier;
    return sp<Expectation>::make(sp<Fence>::make(self, a1, notifier), std::move(notifier));
}

sp<Numeric> NumericUtil::pursue(float s0, const sp<Numeric>& target, float duration, const sp<Numeric>& t)
{
    return sp<Stalker>::make(t ? t : Ark::instance().clock()->duration(), target, s0, duration);
}

sp<Numeric> NumericUtil::vibrate(float s0, float v0, float s1, float v1, float duration, const sp<Numeric>& t)
{
    DCHECK(duration > 0, "Duration must be greater than zero");
    float o, a, t0, t1;
    Math::vibrate(s0, v0, s1, v1, o, a, t0, t1);
    float multiplier = (t1 - t0) / duration;
    const sp<Numeric> b = sp<Numeric::Const>::make(t1 - t0);
    return sp<Vibrate>::make(boundary(mul(t ? t : Ark::instance().clock()->duration(), multiplier), b)->delegate(), a, t0, o);
}

sp<Numeric> NumericUtil::integral(const sp<Numeric>& self, const sp<Numeric>& t)
{
    return sp<Integral>::make(self, t ? t : Ark::instance().clock()->duration());
}

NumericUtil::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(Expression::Compiler<float, NumericOperation<float>>().compile(factory, expr.strip()))
{
    DCHECK(_value, "Numeric expression compile failed: %s", expr.c_str());
}

sp<Numeric> NumericUtil::DICTIONARY::build(const sp<Scope>& args)
{
    return _value->build(args);
}

NumericUtil::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(Expression::Compiler<float, NumericOperation<float>>().compile(factory, Documents::ensureAttribute(manifest, Constants::Attributes::VALUE)))
{
}

sp<Numeric> NumericUtil::BUILDER::build(const sp<Scope>& args)
{
    return _value->build(args);
}

}
