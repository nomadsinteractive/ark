#include "core/util/string_type.h"

#include <regex>
#include <fmt/core.h>

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/base/expression.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_observer.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/inf/array.h"

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

template<typename T> class StringVarFormatedOne : public StringVar {
public:
    StringVarFormatedOne(sp<Variable<T>> value, String format = "")
        : _value(std::move(value)), _format(Strings::sprintf("{%s}", format.c_str())), _formatted(sp<String>::make()) {
    }

    bool update(uint64_t timestamp) override {
        return _value->update(timestamp);
    }

    sp<String> val() override {
        if constexpr(std::is_same_v<T, sp<String>>)
            *_formatted = fmt::format(_format.c_str(), _value->val()->c_str());
        else
            *_formatted = fmt::format(_format.c_str(), _value->val());
        return _formatted;
    }

private:
    sp<Variable<T>> _value;
    String _format;
    sp<String> _formatted;
};

class StringVarList : public StringVar {
public:
    StringVarList(std::vector<sp<StringVar>> list)
        : _list(std::move(list)), _string(sp<String>::make()) {
        update(0);
    }

    virtual bool update(uint64_t timestamp) override {
        bool dirty = false;
        for(const sp<StringVar>& i : _list)
            dirty = i->update(timestamp) | dirty;
        return dirty;
    }

    virtual sp<String> val() override {
        StringBuffer sb;
        for(const sp<StringVar>& i : _list)
            sb << *i->val();
        *_string = sb.str();
        return _string;
    }

private:
    std::vector<sp<StringVar>> _list;
    sp<String> _string;
};

template<typename T, typename... ARGS> sp<StringVar> toStringVar(const Box& box, String format) {
    sp<Variable<T>> var = box.as<Variable<T>>();
    if(var)
        return sp<StringVarFormatedOne<T>>::make(std::move(var), std::move(format));

    if constexpr(sizeof...(ARGS) > 0)
        return toStringVar<ARGS...>(box, std::move(format));

    return nullptr;
}

}

sp<StringVar> StringType::create(sp<String> value)
{
    return sp<StringVar::Impl>::make(std::move(value));
}

sp<StringVar> StringType::create(sp<Integer> value)
{
    return sp<StringVarInteger>::make(std::move(value));
}

sp<StringVar> StringType::create(sp<StringVar> value)
{
    return sp<StringVarWrapper>::make(std::move(value));
}

sp<StringVar> StringType::create()
{
    return sp<StringVar::Impl>::make(sp<String>());
}

String StringType::val(const sp<StringVar>& self)
{
    return self->val();
}

sp<StringVar> StringType::wrapped(const sp<StringVar>& self)
{
    const sp<StringVarWrapper>& ib = self.tryCast<StringVarWrapper>();
    DCHECK_WARN(ib, "Non-TextWrapper instance has no wrapped attribute. This should be an error unless you're inspecting it.");
    return ib ? ib->wrapped() : nullptr;
}

void StringType::set(const sp<StringVar::Impl>& self, sp<String> value)
{
    self->set(std::move(value));
}

void StringType::set(const sp<StringVarWrapper>& self, sp<String> value)
{
    self->set(std::move(value));
}

void StringType::set(const sp<StringVarWrapper>& self, sp<StringVar> value)
{
    self->set(std::move(value));
}

sp<StringVar> StringType::ifElse(sp<StringVar> self, sp<Boolean> condition, sp<StringVar> negative)
{
    return sp<VariableTernary<sp<String>>>::make(std::move(condition), std::move(self), std::move(negative));
}

sp<StringVar> StringType::freeze(const sp<StringVar>& self)
{
    return create(self->val());
}

sp<StringVar> StringType::format(String format, const Scope& kwargs)
{
    static std::regex VAR_PATTERN("\\{(\\w+)(\\:[^}]+)?\\}");
    std::vector<sp<StringVar>> strList;
    format.search(VAR_PATTERN, [&strList, &kwargs] (const std::smatch& matched) {
        const std::string name = matched[1].str();
        if(name.at(1) == '{') {
            strList.push_back(sp<StringVar::Const>::make(sp<String>::make(name)));
            return true;
        }

        const std::string format = matched[2].str();
        const Box value = kwargs.getObject(name);
        CHECK(value, "Unable to get keyword name \"%s\"", name.c_str());
        sp<StringVar> formatted = toStringVar<int32_t, float, sp<String>>(value, format.c_str());
        CHECK(formatted, "Unable to format key \"%s\"", name.c_str());
        strList.push_back(std::move(formatted));
        return true;
    }, [&strList] (const String& unmatched) {
        strList.push_back(sp<StringVar::Const>::make(sp<String>::make(unmatched)));
        return true;
    });

    return sp<StringVarList>::make(std::move(strList));
}

sp<StringVar> StringType::dye(sp<StringVar> self, sp<Boolean> condition, String message)
{
    return sp<VariableDyed<sp<String>>>::make(std::move(self), std::move(condition), std::move(message));
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
    : _value(factory.ensureBuilder<String>(manifest, constants::VALUE)) {
}

sp<StringVar> StringType::BUILDER::build(const Scope& args)
{
    return StringType::create(_value->build(args));
}

}
