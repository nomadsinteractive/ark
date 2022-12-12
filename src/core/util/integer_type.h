#ifndef ARK_CORE_UTIL_INTEGER_TYPE_H_
#define ARK_CORE_UTIL_INTEGER_TYPE_H_

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/expectation_i.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Integer")]]
class ARK_API IntegerType final {
public:
//  [[script::bindings::enumeration]]
    enum Repeat {
        REPEAT_NONE = 0,
        REPEAT_REVERSE = 1,
        REPEAT_ACTION_MASK = 3,
        REPEAT_LOOP = 4,
        REPEAT_LAST = 8,
        REPEAT_NOTIFY = 16
    };

//[[script::bindings::constructor]]
    static sp<Integer> create(int32_t value);
//[[script::bindings::constructor]]
    static sp<Integer> create(const sp<Integer>& value);
//[[script::bindings::constructor]]
    static sp<Integer> create(const sp<Numeric>& value);

//[[script::bindings::operator(+)]]
    static sp<Integer> add(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Integer> sub(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Integer> mul(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(%)]]
    static sp<Integer> mod(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Integer> floordiv(const sp<Integer>& self, const sp<Integer>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Integer> negative(sp<Integer> self);
//[[script::bindings::operator(int)]]
    static int32_t toInt32(const sp<Integer>& self);
//[[script::bindings::operator(float)]]
    static float toFloat(const sp<Integer>& self);

//[[script::bindings::operator(>)]]
    static sp<Boolean> gt(const sp<Integer>& self, const sp<Integer>& other);
//[[script::bindings::operator(>=)]]
    static sp<Boolean> ge(const sp<Integer>& self, const sp<Integer>& other);
//[[script::bindings::operator(<)]]
    static sp<Boolean> lt(const sp<Integer>& self, const sp<Integer>& other);
//[[script::bindings::operator(<=)]]
    static sp<Boolean> le(const sp<Integer>& self, const sp<Integer>& other);
//[[script::bindings::operator(==)]]
    static sp<Boolean> eq(const sp<Integer>& self, const sp<Integer>& other);
//[[script::bindings::operator(!=)]]
    static sp<Boolean> ne(const sp<Integer>& self, const sp<Integer>& other);

//[[script::bindings::property]]
    static int32_t val(const sp<Integer>& self);
//[[script::bindings::property]]
    static void setVal(const sp<Integer::Impl>& self, int32_t value);
//[[script::bindings::property]]
    static void setVal(const sp<IntegerWrapper>& self, int32_t value);
//[[script::bindings::property]]
    static const sp<Integer>& delegate(const sp<Integer>& self);
//[[script::bindings::property]]
    static void setDelegate(const sp<Integer>& self, const sp<Integer>& delegate);

//[[script::bindings::auto]]
    static int32_t toRepeat(const String& repeat);
//[[script::bindings::auto]]
    static sp<ExpectationI> repeat(std::vector<int32_t> array, IntegerType::Repeat repeat = IntegerType::REPEAT_NONE);

//[[script::bindings::classmethod]]
    static void set(const sp<Integer::Impl>& self, int32_t value);
//[[script::bindings::classmethod]]
    static void set(const sp<IntegerWrapper>& self, int32_t value);
//[[script::bindings::classmethod]]
    static void set(const sp<IntegerWrapper>& self, const sp<Integer>& delegate);

//[[script::bindings::classmethod]]
    static void fix(const sp<Integer>& self);
//[[script::bindings::classmethod]]
    static sp<Integer> wrap(const sp<Integer>& self);

//[[script::bindings::classmethod]]
    static sp<Integer> animate(const sp<Integer>& self, const sp<Numeric>& interval = nullptr, const sp<Numeric>& duration = nullptr);

//  [[script::bindings::classmethod]]
    static sp<ExpectationI> atLeast(sp<Integer> self, sp<Integer> a1);
//  [[script::bindings::classmethod]]
    static sp<ExpectationI> atMost(sp<Integer> self, sp<Integer> a1);

//  [[script::bindings::classmethod]]
    static sp<ExpectationI> clamp(const sp<Integer>& self, const sp<Integer>& min, const sp<Integer>& max);
//  [[script::bindings::classmethod]]
    static sp<ExpectationI> fence(const sp<Integer>& self, const sp<Integer>& a1);

//  [[script::bindings::classmethod]]
    static sp<Integer> ifElse(const sp<Integer>& self, const sp<Boolean>& condition, const sp<Integer>& negative);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Integer> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);
        DICTIONARY(BeanFactory& factory, const String& value, Repeat repeat);

        virtual sp<Integer> build(const Scope& args) override;

    private:
        sp<Builder<Integer>> makeIntegerBuilder(BeanFactory& factory, const String& expr, Repeat repeat) const;

    private:
        sp<Builder<Integer>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Integer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Integer> build(const Scope& args) override;

    private:
        DICTIONARY _delegate;
    };

};

}

#endif
