#include "core/util/boolean_type.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/base/expression.h"
#include "core/impl/variable/variable_observer.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"

namespace ark {

namespace {

template<typename OP, typename T = Boolean> class OperationBuilder : public Builder<Boolean> {
public:
    OperationBuilder(const sp<Builder<T>>& a1, const sp<Builder<T>>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual sp<Boolean> build(const Scope& args) override {
        return sp<OP>::make(_a1->build(args), _a2->build(args));
    }

private:
    sp<Builder<T>> _a1;
    sp<Builder<T>> _a2;
};

class NegativeBooleanBuilder : public Builder<Boolean> {
public:
    NegativeBooleanBuilder(const sp<Builder<Boolean>>& a1)
        : _a1(a1) {
    }

    virtual sp<Boolean> build(const Scope& args) override {
        return BooleanUtil::negative(_a1->build(args));
    }

private:
    sp<Builder<Boolean>> _a1;
};

class BooleanOperation {
public:
    template <typename T> using OP2 = VariableOP2<sp<Numeric>, sp<Numeric>, T>;

    static bool isConstant(const String& expr) {
        return expr == "true" || expr == "false";
    }

    static sp<Builder<Boolean>> booleanAnd(const sp<Builder<Boolean>>& a1, const sp<Builder<Boolean>>& a2) {
        return sp<OperationBuilder<VariableOP2<sp<Boolean>, sp<Boolean>, Operators::And<bool>>>>::make(a1, a2);
    }

    static sp<Builder<Boolean>> booleanOr(const sp<Builder<Boolean>>& a1, const sp<Builder<Boolean>>& a2) {
        return sp<OperationBuilder<VariableOP2<sp<Boolean>, sp<Boolean>, Operators::Or<bool>>>>::make(a1, a2);
    }

    static sp<Builder<Boolean>> eval(BeanFactory& factory, const String& expr) {
        static std::regex PATTERN("([\\-+\\s\\w\\d_$@.]+)\\s+([><]=?)\\s+([\\-+\\s\\w\\d_$@.]+)");
        const array<String> matches = expr.match(PATTERN);
        if(!matches) {
            for(const String& i : NEGATIVE_OPS)
                if(expr.startsWith(i))
                    return sp<NegativeBooleanBuilder>::make(eval(factory, expr.substr(i.length())));

            const Identifier id = Identifier::parse(expr);
            if(id.isRef())
                return factory.getBuilderByRef<Boolean>(id);
            if(id.isArg())
                return factory.getBuilderByArg<Boolean>(id.arg());
        }
        DCHECK(matches && matches->length() == 4, "Illegal expression: \"%s\" syntax error", expr.c_str());
        const String* ptr = matches->buf();
        const sp<Builder<Numeric>> lvalue = factory.ensureBuilder<Numeric>(ptr[1]);
        const String& op = ptr[2];
        const sp<Builder<Numeric>> rvalue = factory.ensureBuilder<Numeric>(ptr[3]);
        if(op == ">")
            return sp<OperationBuilder<OP2<Operators::GT<float>>, Numeric>>::make(lvalue, rvalue);
        if(op == "<")
            return sp<OperationBuilder<OP2<Operators::GT<float>>, Numeric>>::make(lvalue, rvalue);
        if(op == ">=")
            return sp<OperationBuilder<OP2<Operators::GE<float>>, Numeric>>::make(lvalue, rvalue);
        if(op == "<=")
            return sp<OperationBuilder<OP2<Operators::LE<float>>, Numeric>>::make(lvalue, rvalue);
        return nullptr;
    }

    static Expression::Operator<bool> OPS[2];
    static String NEGATIVE_OPS[2];

};

Expression::Operator<bool> BooleanOperation::OPS[2] = {
    {"&&", 2, BooleanOperation::booleanAnd},
    {"||", 1, BooleanOperation::booleanOr}
};

String BooleanOperation::NEGATIVE_OPS[2] = {"!", "not "};


static bool _operator_not(bool val)
{
    return !val;
}

}

sp<Boolean> BooleanUtil::create(const sp<Boolean>& value)
{
    return sp<BooleanWrapper>::make(value);
}

sp<Boolean> BooleanUtil::create(bool value)
{
    return sp<BooleanWrapper>::make(value);
}

sp<Boolean> BooleanUtil::__and__(const sp<Boolean>& self, const sp<Boolean>& rvalue)
{
    return sp<VariableOP2<sp<Boolean>, sp<Boolean>, Operators::And<bool>>>::make(self, rvalue);
}

sp<Boolean> BooleanUtil::__or__(const sp<Boolean>& self, const sp<Boolean>& rvalue)
{
    return sp<VariableOP2<sp<Boolean>, sp<Boolean>, Operators::Or<bool>>>::make(self, rvalue);
}

sp<Boolean> BooleanUtil::negative(const sp<Boolean>& self)
{
    return sp<VariableOP1<bool>>::make(_operator_not, self);
}

bool BooleanUtil::toBool(const sp<Boolean>& self)
{
    return self->val();
}

bool BooleanUtil::val(const sp<Boolean>& self)
{
    return self->val();
}

void BooleanUtil::setVal(const sp<Boolean::Impl>& self, bool value)
{
    self->set(value);
}

void BooleanUtil::setVal(const sp<BooleanWrapper>& self, bool value)
{
    self->set(value);
}

const sp<Boolean>& BooleanUtil::delegate(const sp<Boolean>& self)
{
    const sp<BooleanWrapper> ib = self.as<BooleanWrapper>();
    DWARN(ib, "Non-BooleanWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return ib ? ib->delegate() : sp<Boolean>::null();
}

void BooleanUtil::setDelegate(const sp<Boolean>& self, const sp<Boolean>& delegate)
{
    const sp<BooleanWrapper> ib = self.as<BooleanWrapper>();
    DCHECK(ib, "Must be a BooleanWrapper instance to set its delegate attribute");
    ib->set(delegate);
}

void BooleanUtil::set(const sp<Boolean::Impl>& self, bool value)
{
    self->set(value);
}

void BooleanUtil::set(const sp<BooleanWrapper>& self, bool value)
{
    self->set(value);
}

void BooleanUtil::set(const sp<BooleanWrapper>& self, const sp<Boolean>& delegate)
{
    self->set(delegate);
}

void BooleanUtil::toggle(const sp<Boolean::Impl>& self)
{
    self->set(!self->val());
}

void BooleanUtil::toggle(const sp<BooleanWrapper>& self)
{
    self->set(!self->val());
}

sp<Boolean> BooleanUtil::observe(const sp<Boolean>& self, const sp<Observer>& observer)
{
    return sp<VariableObserver<bool>>::make(self, observer);
}

sp<Numeric> BooleanUtil::ternary(const sp<Boolean>& self, const sp<Numeric>& postive, const sp<Numeric>& negative)
{
    return sp<VariableTernary<float>>::make(self, postive, negative);
}

sp<Vec2> BooleanUtil::ternary(const sp<Boolean>& self, const sp<Vec2>& postive, const sp<Vec2>& negative)
{
    return sp<VariableTernary<V2>>::make(self, postive, negative);
}

sp<Vec3> BooleanUtil::ternary(const sp<Boolean>& self, const sp<Vec3>& postive, const sp<Vec3>& negative)
{
    return sp<VariableTernary<V3>>::make(self, postive, negative);
}

void BooleanUtil::fix(const sp<Boolean>& self)
{
    const sp<BooleanWrapper> ib = self.as<BooleanWrapper>();
    DWARN(ib, "Calling fix on non-BooleanWrapper has no effect.");
    if(ib)
        ib->fix();
}

BooleanUtil::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(Expression::Compiler<bool, BooleanOperation>().compile(factory, expr.strip()))
{
    DCHECK(_value, "Boolean expression compile failed: %s", expr.c_str());
}

sp<Boolean> BooleanUtil::DICTIONARY::build(const Scope& args)
{
    return _value->build(args);
}

BooleanUtil::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(Expression::Compiler<bool, BooleanOperation>().compile(factory, getValue(manifest))) {
}

sp<Boolean> BooleanUtil::BUILDER::build(const Scope& args)
{
    return _value->build(args);
}

String BooleanUtil::BUILDER::getValue(const document& manifest) const
{
    DCHECK(!manifest->getAttribute(Constants::Attributes::CLASS), "Document \"%s\" has class named \"%s\", which cannot been built into a Boolean object",
           Documents::toString(manifest).c_str(), manifest->getAttribute(Constants::Attributes::CLASS)->value().c_str());
    return Documents::ensureAttribute(manifest, Constants::Attributes::VALUE).strip();
}

}
