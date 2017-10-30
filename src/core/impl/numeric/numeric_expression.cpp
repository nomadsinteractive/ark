#include "core/impl/numeric/numeric_expression.h"

#include "core/base/bean_factory.h"
#include "core/base/expression.h"
#include "core/impl/numeric/add.h"
#include "core/impl/numeric/divide.h"
#include "core/impl/numeric/multiply.h"
#include "core/impl/numeric/subtract.h"
#include "core/util/bean_utils.h"

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
        return sp<OperationBuilder<Add>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> subtract(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<Subtract>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> multiply(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<Multiply>>::make(a1, a2);
    }

    static sp<Builder<Numeric>> divide(const sp<Builder<Numeric>>& a1, const sp<Builder<Numeric>>& a2) {
        return sp<OperationBuilder<Divide>>::make(a1, a2);
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

NumericExpression::DICTIONARY::DICTIONARY(BeanFactory& parent, const String& value)
    : _expression(Expression::Compiler<float, NumericOperation>().compile(parent, value.strip()))
{
}

sp<Numeric> NumericExpression::DICTIONARY::build(const sp<Scope>& args)
{
    return _expression->build(args);
}

NumericExpression::BUILDER::BUILDER(BeanFactory& parent, const document& doc)
    : _expression(Expression::Compiler<float, NumericOperation>().compile(parent, Documents::ensureAttribute(doc, Constants::Attributes::VALUE)))
{
}

sp<Numeric> NumericExpression::BUILDER::build(const sp<Scope>& args)
{
    return _expression->build(args);
}

NumericExpression::NUMERIC_BUILDER::NUMERIC_BUILDER(BeanFactory& parent, const document& doc)
    : _expression(Expression::Compiler<float, NumericOperation>().compile(parent, Documents::ensureAttribute(doc, Constants::Attributes::VALUE)))
{
}

sp<Numeric> NumericExpression::NUMERIC_BUILDER::build(const sp<Scope>& args)
{
    return sp<Numeric::Impl>::make(_expression->build(args)->val());
}

}
