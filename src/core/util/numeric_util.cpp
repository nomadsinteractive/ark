#include "core/util/numeric_util.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/expression.h"
#include "core/impl/builder/builder_by_instance.h"
#include "core/impl/numeric/expect.h"
#include "core/impl/numeric/max.h"
#include "core/impl/numeric/min.h"
#include "core/impl/numeric/numeric_depends.h"
#include "core/impl/numeric/numeric_negative.h"
#include "core/impl/numeric/stalker.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/util/strings.h"
#include "core/util/operators.h"

namespace ark {

namespace {

template<typename OP> class OperationBuilder : public Builder<Numeric> {
public:
    OperationBuilder(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual sp<Numeric> build(const sp<Scope>& args) override {
        return sp<VariableOP2<float, float, OP, sp<Numeric>, sp<Numeric>>>::make(_a1->build(args), _a2->build(args));
    }

private:
    sp<Builder<Numeric>> _a1;
    sp<Builder<Numeric>> _a2;
};

class NumericOperation {
public:
    static bool isConstant(const String& expr) {
        return Strings::isNumeric(expr);
    }

    static sp<Builder<Numeric>> add(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<Operators::Add<float>>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> subtract(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<Operators::Sub<float>>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> multiply(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<Operators::Mul<float>>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> divide(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<Operators::Div<float>>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> eval(BeanFactory& /*factory*/, const String& expr) {
        return sp<BuilderByInstance<Numeric>>::make(sp<Numeric::Const>::make(Strings::parse<float>(expr)));
    }

    static Expression::Operator<float> OPS[4];

};

Expression::Operator<float> NumericOperation::OPS[4] = {
    {"+", 1, NumericOperation::add},
    {"-", 1, NumericOperation::subtract},
    {"*", 2, NumericOperation::multiply},
    {"/", 2, NumericOperation::divide}
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

void NumericUtil::iadd(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    FATAL("Unimplemented");
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

void NumericUtil::isub(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    FATAL("Unimplemented");
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

void NumericUtil::imul(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    FATAL("Unimplemented");
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

sp<Numeric> NumericUtil::negative(const sp<Numeric>& self)
{
    return sp<NumericNegative>::make(self);
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
    DCHECK(nw, "Must be an NumericWrapper instance to get its delegate attribute");
    return nw->delegate();
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

sp<Numeric> NumericUtil::depends(const sp<Numeric>& self, const sp<Numeric>& depends)
{
    return sp<NumericDepends>::make(self, depends);
}

sp<Numeric> NumericUtil::boundary(const sp<Numeric>& self, const sp<Numeric>& a2)
{
    return self->val() < a2->val() ? sp<Numeric>::adopt(new Min(self, a2)) : sp<Numeric>::adopt(new Max(self, a2));
}

sp<Numeric> NumericUtil::expect(const sp<Numeric>& self, const sp<Expectation>& expectation)
{
    return sp<Expect>::make(self, expectation);
}

sp<Numeric> NumericUtil::ternary(const sp<Numeric>& self, const sp<Boolean>& condition, const sp<Numeric>& other)
{
    return sp<VariableTernary<float>>::make(condition, self, other);
}

sp<Numeric> NumericUtil::makeStalker(const sp<Numeric>& self, float s0, float eta, const sp<Numeric>& duration)
{
    return sp<Stalker>::make(duration ? duration : Ark::instance().clock()->duration(), self, s0, eta);
}

NumericUtil::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& manifest)
    : _value(Expression::Compiler<float, NumericOperation>().compile(factory, manifest.strip()))
{
}

sp<Numeric> NumericUtil::DICTIONARY::build(const sp<Scope>& args)
{
    return _value->build(args);
}

NumericUtil::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(Expression::Compiler<float, NumericOperation>().compile(factory, Documents::ensureAttribute(manifest, Constants::Attributes::VALUE)))
{
}

sp<Numeric> NumericUtil::BUILDER::build(const sp<Scope>& args)
{
    return _value->build(args);
}

}
