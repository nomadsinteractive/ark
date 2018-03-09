#include "core/util/boolean_util.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/base/expression.h"
#include "core/impl/boolean/boolean_and.h"
#include "core/impl/boolean/boolean_not.h"
#include "core/impl/boolean/boolean_or.h"

namespace ark {

namespace {

class GT : public Boolean {
public:
    GT(const sp<Numeric>& a1, const sp<Numeric>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual bool val() override {
        return _a1->val() > _a2->val();
    }

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

class LT : public Boolean {
public:
    LT(const sp<Numeric>& a1, const sp<Numeric>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual bool val() override {
        return _a1->val() < _a2->val();
    }

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

class GET : public Boolean {
public:
    GET(const sp<Numeric>& a1, const sp<Numeric>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual bool val() override {
        return _a1->val() >= _a2->val();
    }

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

class LET : public Boolean {
public:
    LET(const sp<Numeric>& a1, const sp<Numeric>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual bool val() override {
        return _a1->val() <= _a2->val();
    }

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

template<typename OP, typename T = Boolean> class OperationBuilder : public Builder<Boolean> {
public:
    OperationBuilder(const sp<Builder<T>>& a1, const sp<Builder<T>>& a2)
        : _a1(a1), _a2(a2) {
    }

    virtual sp<Boolean> build(const sp<Scope>& args) override {
        return sp<OP>::make(_a1->build(args), _a2->build(args));
    }

private:
    sp<Builder<T>> _a1;
    sp<Builder<T>> _a2;
};

class BooleanOperation {
public:
    static bool isConstant(const String& expr) {
        return expr == "true" || expr == "false";
    }

    static sp<Builder<Boolean>> booleanAnd(const sp<Builder<Boolean>>& a1, const sp<Builder<Boolean>>& a2) {
        return sp<OperationBuilder<BooleanAnd>>::make(a1, a2);
    }

    static sp<Builder<Boolean>> booleanOr(const sp<Builder<Boolean>>& a1, const sp<Builder<Boolean>>& a2) {
        return sp<OperationBuilder<BooleanOr>>::make(a1, a2);
    }

    static sp<Builder<Boolean>> eval(BeanFactory& factory, const String& expr) {
        static std::regex PATTERN("([\\-+\\s\\w\\d_$@.]+)\\s+([><]=?)\\s+([\\-+\\s\\w\\d_$@.]+)");
        const array<String> matches = expr.match(PATTERN);
        DCHECK(matches && matches->length() == 4, "Illegal expression: \"%s\" syntax error", expr.c_str());
        const String* ptr = matches->array();
        const sp<Builder<Numeric>> lvalue = factory.ensureBuilder<Numeric>(ptr[1]);
        const String& op = ptr[2];
        const sp<Builder<Numeric>> rvalue = factory.ensureBuilder<Numeric>(ptr[3]);
        if(op == ">")
            return sp<OperationBuilder<GT, Numeric>>::make(lvalue, rvalue);
        if(op == "<")
            return sp<OperationBuilder<LT, Numeric>>::make(lvalue, rvalue);
        if(op == ">=")
            return sp<OperationBuilder<GET, Numeric>>::make(lvalue, rvalue);
        if(op == "<=")
            return sp<OperationBuilder<LET, Numeric>>::make(lvalue, rvalue);
        return nullptr;
    }

    static Expression::Operator<bool> OPS[2];

};

Expression::Operator<bool> BooleanOperation::OPS[2] = {
    {"and", 2, BooleanOperation::booleanAnd},
    {"or", 1, BooleanOperation::booleanOr}
};

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
    return sp<BooleanAnd>::make(self, rvalue);
}

sp<Boolean> BooleanUtil::__or__(const sp<Boolean>& self, const sp<Boolean>& rvalue)
{
    return sp<BooleanOr>::make(self, rvalue);
}

sp<Boolean> BooleanUtil::negative(const sp<Boolean>& self)
{
    return sp<BooleanNot>::make(self);
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
    DCHECK(ib, "Must be an BooleanWrapper instance to get its delegate attribute");
    return ib->delegate();
}

void BooleanUtil::setDelegate(const sp<Boolean>& self, const sp<Boolean>& delegate)
{
    const sp<BooleanWrapper> ib = self.as<BooleanWrapper>();
    DCHECK(ib, "Must be an BooleanWrapper instance to set its delegate attribute");
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

void BooleanUtil::fix(const sp<Boolean>& self)
{
    const sp<BooleanWrapper> ib = self.as<BooleanWrapper>();
    DWARN(ib, "Calling fix on non-BooleanWrapper has no effect.");
    if(ib)
        ib->fix();
}

BooleanUtil::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& value)
    : _value(Expression::Compiler<bool, BooleanOperation>().compile(factory, value.strip()))
{
}

sp<Boolean> BooleanUtil::DICTIONARY::build(const sp<Scope>& args)
{
    return _value->build(args);
}

BooleanUtil::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(Expression::Compiler<bool, BooleanOperation>().compile(factory, Documents::ensureAttribute(manifest, Constants::Attributes::VALUE).strip())) {
}

sp<Boolean> BooleanUtil::BUILDER::build(const sp<Scope>& args)
{
    return _value->build(args);
}

}
