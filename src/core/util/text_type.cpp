#include "core/util/text_type.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/base/expression.h"
#include "core/impl/variable/variable_observer.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"

namespace ark {

sp<Text> TextType::create(sp<Text> value)
{
    return sp<TextWrapper>::make(std::move(value));
}

sp<Text> TextType::create(sp<String> value)
{
    return sp<Text::Impl>::make(std::move(value));
}

String TextType::val(const sp<Text>& self)
{
    return self->val();
}

const sp<Text>& TextType::delegate(const sp<Text>& self)
{
    const sp<TextWrapper> ib = self.as<TextWrapper>();
    DWARN(ib, "Non-TextWrapper instance has no delegate attribute. This should be an error unless you're inspecting it.");
    return ib ? ib->delegate() : sp<Text>::null();
}

void TextType::setDelegate(const sp<Text>& self, const sp<Text>& delegate)
{
    const sp<TextWrapper> ib = self.as<TextWrapper>();
    DCHECK(ib, "Must be a TextWrapper instance to set its delegate attribute");
    ib->set(delegate);
}

void TextType::set(const sp<Text::Impl>& self, sp<String> value)
{
    self->set(std::move(value));
}

void TextType::set(const sp<TextWrapper>& self, sp<String> value)
{
    self->set(std::move(value));
}

void TextType::set(const sp<TextWrapper>& self, sp<Text> delegate)
{
    self->set(std::move(delegate));
}

void TextType::fix(const sp<Text>& self)
{
    const sp<TextWrapper> ib = self.as<TextWrapper>();
    DWARN(ib, "Calling fix on non-BooleanWrapper has no effect.");
    if(ib)
        ib->fix();
}

TextType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(factory.ensureBuilder<String>(expr))
{
}

sp<Text> TextType::DICTIONARY::build(const Scope& args)
{
    return TextType::create(_value->build(args));
}

TextType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(factory.ensureBuilder<String>(manifest, Constants::Attributes::VALUE)) {
}

sp<Text> TextType::BUILDER::build(const Scope& args)
{
    return TextType::create(_value->build(args));
}

}
