#ifndef ARK_CORE_UTIL_TEXT_TYPE_H_
#define ARK_CORE_UTIL_TEXT_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::class("Text")]]
class ARK_API TextType {
public:
//[[script::bindings::constructor]]
    static sp<Text> create(sp<Text> value);
//[[script::bindings::constructor]]
    static sp<Text> create(sp<String> value);
    static sp<Text> create();

//[[script::bindings::property]]
    static String val(const sp<Text>& self);
//[[script::bindings::property]]
    static const sp<Text>& delegate(const sp<Text>& self);
//[[script::bindings::property]]
    static void setDelegate(const sp<Text>& self, const sp<Text>& delegate);

//[[script::bindings::classmethod]]
    static void set(const sp<Text::Impl>& self, sp<String> value);
//[[script::bindings::classmethod]]
    static void set(const sp<TextWrapper>& self, sp<String> value);
//[[script::bindings::classmethod]]
    static void set(const sp<TextWrapper>& self, sp<Text> delegate);

//[[script::bindings::classmethod]]
    static void fix(const sp<Text>& self);

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Text> {
    public:
        DICTIONARY(BeanFactory&, const String& expr);

        virtual sp<Text> build(const Scope& args) override;

    private:
        sp<Builder<String>> _value;
    };

//  [[plugin::builder]]
    class BUILDER : public Builder<Text> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Text> build(const Scope& args) override;

    private:
        String getValue(const document& manifest) const;

    private:
        sp<Builder<String>> _value;
    };
};

}

#endif
