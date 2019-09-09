#include "core/util/integer_util.h"

#include "core/base/bean_factory.h"
#include "core/base/clock.h"
#include "core/base/expression.h"
#include "core/impl/integer/integer_by_array.h"
#include "core/impl/integer/integer_by_interval.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/impl/variable/variable_op2.h"
#include "core/util/operators.h"
#include "core/util/strings.h"

namespace ark {

namespace {

class IntegerArray : public Integer, public IntArray, public Implements<IntegerArray, Integer, IntArray> {
public:
    IntegerArray(std::vector<int32_t> values)
        : _values(std::move(values)), _iter(0) {
    }

    virtual int32_t val() override {
        return _iter < _values.size() ? _values.at(_iter++) : -1;
    }

    virtual size_t length() override {
        return _values.size();
    }

    virtual int32_t* buf() override {
        return &_values[0];
    }

    class BUILDER : public Builder<Integer> {
    public:
        BUILDER(std::vector<int32_t> values)
            : _values(std::move(values)) {
        }

        virtual sp<Integer> build(const sp<Scope>& /*args*/) override {
            return sp<IntegerArray>::make(_values);
        }

    private:
        std::vector<int32_t> _values;
    };

private:
    std::vector<int32_t> _values;
    size_t _iter;
};

}

sp<Integer> IntegerUtil::create(const sp<Integer>& value)
{
    return sp<IntegerWrapper>::make(value);
}

sp<Integer> IntegerUtil::create(int32_t value)
{
    return sp<IntegerWrapper>::make(value);
}

sp<Integer> IntegerUtil::create(std::vector<int32_t> values)
{
    return sp<IntegerArray>::make(std::move(values));
}

sp<Integer> IntegerUtil::add(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::Add<int32_t>, sp<Integer>, sp<Integer>>>::make(self, rvalue);
}

sp<Integer> IntegerUtil::sub(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::Sub<int32_t>, sp<Integer>, sp<Integer>>>::make(self, rvalue);
}

sp<Integer> IntegerUtil::mul(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::Mul<int32_t>, sp<Integer>, sp<Integer>>>::make(self, rvalue);
}

sp<Integer> IntegerUtil::mod(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Numeric> IntegerUtil::truediv(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    FATAL("Unimplemented");
    return nullptr;
}

sp<Integer> IntegerUtil::floordiv(const sp<Integer>& self, const sp<Integer>& rvalue)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::Div<int32_t>, sp<Integer>, sp<Integer>>>::make(self, rvalue);
}

sp<Integer> IntegerUtil::negative(const sp<Integer>& self)
{
    FATAL("Unimplemented");
    return nullptr;
}

int32_t IntegerUtil::toInt32(const sp<Integer>& self)
{
    return self->val();
}

float IntegerUtil::toFloat(const sp<Integer>& self)
{
    return static_cast<float>(self->val());
}

sp<Boolean> IntegerUtil::gt(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::GT<int32_t>, sp<Integer>, sp<Integer>>>::make(self, other);
}

sp<Boolean> IntegerUtil::ge(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::GE<int32_t>, sp<Integer>, sp<Integer>>>::make(self, other);
}

sp<Boolean> IntegerUtil::lt(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::LT<int32_t>, sp<Integer>, sp<Integer>>>::make(self, other);
}

sp<Boolean> IntegerUtil::le(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::LE<int32_t>, sp<Integer>, sp<Integer>>>::make(self, other);
}

sp<Boolean> IntegerUtil::eq(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::EQ<int32_t>, sp<Integer>, sp<Integer>>>::make(self, other);
}

sp<Boolean> IntegerUtil::ne(const sp<Integer>& self, const sp<Integer>& other)
{
    return sp<VariableOP2<int32_t, int32_t, Operators::NE<int32_t>, sp<Integer>, sp<Integer>>>::make(self, other);
}

int32_t IntegerUtil::val(const sp<Integer>& self)
{
    return self->val();
}

void IntegerUtil::setVal(const sp<Integer::Impl>& self, int32_t value)
{
    self->set(value);
}

void IntegerUtil::setVal(const sp<IntegerWrapper>& self, int32_t value)
{
    self->set(value);
}

const sp<Integer>& IntegerUtil::delegate(const sp<Integer>& self)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DWARN(iw, "Non-IntegerWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return iw ? iw->delegate() : sp<Integer>::null();
}

void IntegerUtil::setDelegate(const sp<Integer>& self, const sp<Integer>& delegate)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DCHECK(iw, "Must be an IntegerWrapper instance to set its delegate attribute");
    iw->set(delegate);
}

void IntegerUtil::set(const sp<Integer::Impl>& self, int32_t value)
{
    self->set(value);
}

void IntegerUtil::set(const sp<IntegerWrapper>& self, int32_t value)
{
    self->set(value);
}

void IntegerUtil::set(const sp<IntegerWrapper>& self, const sp<Integer>& delegate)
{
    self->set(delegate);
}

void IntegerUtil::fix(const sp<Integer>& self)
{
    const sp<IntegerWrapper> iw = self.as<IntegerWrapper>();
    DWARN(iw, "Calling fix on non-IntegerWrapper has no effect.");
    if(iw)
        iw->fix();
}

sp<Integer> IntegerUtil::wrap(const sp<Integer>& self)
{
    return sp<IntegerWrapper>::make(self);
}

sp<Integer> IntegerUtil::repeat(const sp<Integer>& self, IntegerUtil::Repeat repeat)
{
    const sp<IntArray> ia = self.as<IntArray>();
    DCHECK(ia, "Animation needs an IntArray instance, others have not been implemented yet");
    return sp<IntegerByArray>::make(ia, repeat);
}

sp<Integer> IntegerUtil::animate(const sp<Integer>& self, const sp<Numeric>& interval, const sp<Numeric>& duration)
{
    return sp<IntegerByInterval>::make(self, duration ? duration : Ark::instance().clock()->duration(), interval ? interval : sp<Numeric>::adopt(new Numeric::Const(1.0f / 24)));
}

IntegerUtil::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(makeIntegerBuilder(factory, expr.strip()))
{
}

sp<Integer> IntegerUtil::DICTIONARY::build(const sp<Scope>& args)
{
    return _value->build(args);
}

sp<Builder<Integer>> IntegerUtil::DICTIONARY::makeIntegerBuilder(BeanFactory& factory, const String& expr) const
{
    DCHECK(expr, "Empty Integer expression");
    if(expr.at(0) == '[')
    {
        DCHECK(expr.at(expr.length() - 1) == ']', "Illegal IntArray expression");
        std::vector<int32_t> values;
        for(const String& i : expr.substr(1, expr.length() - 1).split(','))
            values.push_back(Strings::parse<int32_t>(i.strip()));
        return sp<IntegerArray::BUILDER>::make(std::move(values));
    }
    return Expression::Compiler<int32_t, NumericOperation<int32_t>>().compile(factory, expr);
}

IntegerUtil::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _delegate(factory, Documents::ensureAttribute(manifest, Constants::Attributes::VALUE))
{
}

sp<Integer> IntegerUtil::BUILDER::build(const sp<Scope>& args)
{
    return _delegate.build(args);
}

}
