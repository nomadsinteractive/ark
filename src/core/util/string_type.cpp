#include "core/util/string_type.h"

#include <regex>

#include "core/base/bean_factory.h"
#include "core/base/expression.h"
#include "core/impl/variable/variable_observer.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"

namespace ark {

namespace {

class StringVarInteger : public StringVar {
public:
    StringVarInteger(sp<Integer> delegate)
        : _delegate(std::move(delegate)), _text(sp<String>::make(Strings::toString(_delegate->val()))) {
    }

    virtual bool update(uint64_t timestamp) override {
        if(_delegate->update(timestamp)) {
            _text = sp<String>::make(Strings::toString(_delegate->val()));
            return true;
        }
        return false;
    }

    virtual sp<String> val() override {
        return _text;
    }

private:
    sp<Integer> _delegate;
    sp<String> _text;
};


class StringVarFormatted : public StringVar {
public:
    StringVarFormatted(String format, std::map<String, Box> args)
        : _format(format), _args(std::move(args)) {
        update(0);
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const auto& [i, j] : _args)
            dirty = tryUpdate<float, int32_t>(i, j, timestamp) || dirty;
        if(dirty)
            _text = sp<String>::make(doFormat());
        return dirty;
    }

    virtual sp<String> val() override {
        return _text;
    }

private:
    template<typename T, typename... ARGS> bool tryUpdate(const String& name, const Box& box, uint64_t timestamp) {
        sp<Variable<T>> var = box.as<Variable<T>>();
        if(var) {
            bool dirty = timestamp ? var->update(timestamp) : true;
            if(dirty) {
                if constexpr(std::is_same_v<T, sp<String>>)
                    _values[name] = *var->val();
                else
                    _values[name] = Strings::toString<T>(var->val());
            }
            return dirty;
        }
        if constexpr(sizeof...(ARGS) > 0)
            return tryUpdate<ARGS...>(name, box, timestamp);
        return false;
    }

    String doFormat() const {
        static std::regex VAR_PATTERN("\\{(\\w+)\\}");
        return _format.replace(VAR_PATTERN, [this] (Array<String>& matches) {
            const String& keyname = matches.at(1);
            if(keyname[0] == '{')
                return keyname;
            const auto iter = _values.find(keyname);
            CHECK(iter != _values.end(), "Undefined var \"%s\"", keyname.c_str());
            return iter->second;
        });
    }

private:
    String _format;
    std::map<String, Box> _args;
    std::map<String, String> _values;
    sp<String> _text;
};

}

sp<StringVar> StringType::create(sp<StringVar> value)
{
    return sp<StringVarWrapper>::make(std::move(value));
}

sp<StringVar> StringType::create(sp<String> value)
{
    return sp<StringVar::Impl>::make(std::move(value));
}

sp<StringVar> StringType::create(sp<Integer> value)
{
    return sp<StringVarInteger>::make(std::move(value));
}

sp<StringVar> StringType::create()
{
    return sp<StringVar::Impl>::make(sp<String>::null());
}

String StringType::val(const sp<StringVar>& self)
{
    return self->val();
}

const sp<StringVar>& StringType::wrapped(const sp<StringVar>& self)
{
    const sp<StringVarWrapper> ib = self.as<StringVarWrapper>();
    DWARN(ib, "Non-TextWrapper instance has no wrapped attribute. This should be an error unless you're inspecting it.");
    return ib ? ib->wrapped() : sp<StringVar>::null();
}

void StringType::set(const sp<StringVar::Impl>& self, sp<String> value)
{
    self->set(std::move(value));
}

void StringType::set(const sp<StringVarWrapper>& self, sp<String> value)
{
    self->set(std::move(value));
}

void StringType::set(const sp<StringVarWrapper>& self, sp<StringVar> delegate)
{
    self->set(std::move(delegate));
}

sp<StringVar> StringType::freeze(const sp<StringVar>& self)
{
    return create(self->val());
}

sp<StringVar> StringType::format(String format, const Scope& kwargs)
{
    return sp<StringVarFormatted>::make(std::move(format), kwargs.variables());
}

StringType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(factory.ensureBuilder<String>(expr))
{
}

sp<StringVar> StringType::DICTIONARY::build(const Scope& args)
{
    return StringType::create(_value->build(args));
}

StringType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(factory.ensureBuilder<String>(manifest, Constants::Attributes::VALUE)) {
}

sp<StringVar> StringType::BUILDER::build(const Scope& args)
{
    return StringType::create(_value->build(args));
}

}
