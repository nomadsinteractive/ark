#include "core/util/numeric_util.h"

#include "core/base/bean_factory.h"
#include "core/base/expression.h"
#include "core/impl/numeric/numeric_add.h"
#include "core/impl/numeric/numeric_divide.h"
#include "core/impl/numeric/numeric_multiply.h"
#include "core/impl/numeric/numeric_negative.h"
#include "core/impl/numeric/numeric_subtract.h"

namespace ark {

namespace {

template<typename OP> class OperationBuilder : public Builder<Numeric> {
public:
    OperationBuilder(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual sp<Numeric> build(const sp<Scope>& args) override {
        return sp<OP>::make(_a1->build(args), _a2->build(args));
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
        return sp<OperationBuilder<NumericAdd>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> subtract(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<NumericSubtract>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> multiply(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<NumericMultiply>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> divide(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<NumericDivide>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> eval(BeanFactory& /*factory*/, const String& expr) {
        FATAL("Illegal expression: \"%s\" syntax error", expr.c_str());
        return nullptr;
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

sp<Numeric> NumericUtil::add(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    return sp<NumericAdd>::make(self, rvalue);
}

void NumericUtil::iadd(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    FATAL("Unimplemented");
}

sp<Numeric> NumericUtil::sub(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    return sp<NumericSubtract>::make(self, rvalue);
}

void NumericUtil::isub(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    FATAL("Unimplemented");
}

sp<Numeric> NumericUtil::mul(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    return sp<NumericMultiply>::make(self, rvalue);
}

void NumericUtil::imul(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    FATAL("Unimplemented");
}

sp<Numeric> NumericUtil::truediv(const sp<Numeric>& self, const sp<Numeric>& rvalue)
{
    return sp<NumericDivide>::make(self, rvalue);
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

void NumericUtil::fix(const sp<Numeric>& self)
{
    const sp<NumericWrapper> iw = self.as<NumericWrapper>();
    DWARN(iw, "Calling fix on non-NumericWrapper has no effect.");
    if(iw)
        iw->fix();
}

NumericUtil::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _value(Expression::Compiler<float, NumericOperation>().compile(parent, value.strip()))
{
}

sp<Numeric> NumericUtil::DICTIONARY::build(const sp<Scope>& args)
{
    return _value->build(args);
}

NumericUtil::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _value(Expression::Compiler<float, NumericOperation>().compile(parent, Documents::ensureAttribute(doc, Constants::Attributes::VALUE)))
{
}

sp<Numeric> NumericUtil::BUILDER::build(const sp<Scope>& args)
{
    return _value->build(args);
}

}
