#ifndef ARK_CORE_UTIL_NUMERIC_UTIL_H_
#define ARK_CORE_UTIL_NUMERIC_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Numeric")]]
class ARK_API NumericUtil final {
public:
//[[script::bindings::constructor]]
    static sp<Numeric> create(const sp<Numeric>& value);
//[[script::bindings::constructor]]
    static sp<Numeric> create(float value);

//[[script::bindings::operator(+)]]
    static sp<Numeric> add(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(+=)]]
    static void iadd(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(-)]]
    static sp<Numeric> sub(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(-=)]]
    static void isub(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(*)]]
    static sp<Numeric> mul(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(*=)]]
    static void imul(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(/)]]
    static sp<Numeric> truediv(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(//)]]
    static sp<Numeric> floordiv(const sp<Numeric>& self, const sp<Numeric>& rvalue);
//[[script::bindings::operator(neg)]]
    static sp<Numeric> negative(const sp<Numeric>& self);
//[[script::bindings::operator(int)]]
    static int32_t toInt32(const sp<Numeric>& self);
//[[script::bindings::operator(float)]]
    static float toFloat(const sp<Numeric>& self);

//[[script::bindings::property]]
    static float val(const sp<Numeric>& self);
//[[script::bindings::property]]
    static void setVal(const sp<Numeric>& self, float value);
//[[script::bindings::property]]
    static const sp<Numeric>& delegate(const sp<Numeric>& self);
//[[script::bindings::property]]
    static void setDelegate(const sp<Numeric>& self, const sp<Numeric>& delegate);

//[[script::bindings::classmethod]]
    static void set(const sp<Numeric::Impl>& self, float value);
//[[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, float value);
//[[script::bindings::classmethod]]
    static void set(const sp<NumericWrapper>& self, const sp<Numeric>& delegate);

//[[script::bindings::classmethod]]
    static void fix(const sp<Numeric>& self);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Numeric> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _value;
    };

};

}

#endif
