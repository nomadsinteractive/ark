#include "core/util/string_type.h"

#include <regex>
#include <fmt/core.h>

#include "core/base/bean_factory.h"
#include "core/base/constants.h"
#include "core/base/expression.h"
#include "core/impl/variable/variable_dyed.h"
#include "core/impl/variable/variable_op1.h"
#include "core/impl/variable/variable_op2.h"
#include "core/impl/variable/variable_ternary.h"
#include "core/inf/array.h"

namespace ark {

namespace {

class StringVarImpl final : public StringVar, Implements<StringVarImpl, StringVar> {
public:
    StringVarImpl(String value)
        : _value(std::move(value)) {
    }

    bool update(const uint32_t tick) override {
        return _timestamp.update(tick);
    }

    void set(String value)
    {
        _value = std::move(value);
    }

    StringView val() override {
        return _value;
    }

private:
    String _value;
    Timestamp _timestamp;
};

class StringVarInteger final : public StringVar {
public:
    StringVarInteger(sp<Integer> delegate)
        : _delegate(std::move(delegate)), _value(Strings::toString(_delegate->val())) {
    }

    bool update(const uint32_t tick) override {
        if(_delegate->update(tick)) {
            _value = Strings::toString(_delegate->val());
            return true;
        }
        return false;
    }

    StringView val() override {
        return _value;
    }

private:
    sp<Integer> _delegate;
    String _value;
};

template<typename T> class StringVarFormatedOne final : public StringVar {
public:
    StringVarFormatedOne(sp<Variable<T>> value, const StringView format = "")
        : _value(std::move(value)), _format(Strings::sprintf("{%s}", format.data()))
    {
        doUpdate();
    }

    bool update(uint32_t tick) override
    {
        const bool dirty = _value->update(tick);
        if(dirty)
            doUpdate();
        return dirty;
    }

    StringView val() override
    {
        return _formatted;
    }

private:
    void doUpdate()
    {
        if constexpr(std::is_same_v<T, StringView>)
        {
            const char* arg1 = _value->val().data();
            _formatted = fmt::vformat(_format.c_str(), fmt::make_format_args(arg1));
        }
        else
        {
            const T arg1 = _value->val();
            _formatted = fmt::vformat(_format.c_str(), fmt::make_format_args(arg1));
        }
    }

private:
    sp<Variable<T>> _value;
    String _format;
    String _formatted;
};

class StringVarList final : public StringVar {
public:
    StringVarList(Vector<sp<StringVar>> list)
        : _list(std::move(list))
    {
        doUpdate();
    }

    bool update(const uint32_t tick) override {
        bool dirty = false;
        for(const sp<StringVar>& i : _list)
            dirty = i->update(tick) | dirty;
        if(dirty)
            doUpdate();
        return dirty;
    }

    StringView val() override {
        return _value;
    }

private:
    void doUpdate()
    {
        StringBuffer sb;
        for(const sp<StringVar>& i : _list)
            sb << i->val();
        _value = sb.str();
    }

private:
    Vector<sp<StringVar>> _list;
    String _value;
};

template<typename T, typename... ARGS> sp<StringVar> toStringVar(const Box& box, const String& format) {
    if(sp<Variable<T>> var = box.as<Variable<T>>())
        return sp<StringVar>::make<StringVarFormatedOne<T>>(std::move(var), format);

    if constexpr(sizeof...(ARGS) > 0)
        return toStringVar<ARGS...>(box, format);

    return nullptr;
}

}

sp<StringVar> StringType::create(String value)
{
    return sp<StringVar>::make<StringVarWrapper>(sp<StringVar>::make<StringVarImpl>(std::move(value)));
}

sp<StringVar> StringType::create(sp<Integer> value)
{
    return sp<StringVar>::make<StringVarWrapper>(sp<StringVar>::make<StringVarInteger>(std::move(value)));
}

sp<StringVar> StringType::create(sp<StringVar> value)
{
    return sp<StringVar>::make<StringVarWrapper>(std::move(value));
}

sp<StringVar> StringType::create()
{
    return sp<StringVar>::make<StringVarImpl>("");
}

StringView StringType::val(const sp<StringVar>& self)
{
    return self->val();
}

sp<StringVar> StringType::wrapped(const sp<StringVar>& self)
{
    const sp<StringVarWrapper>& ib = self.asInstance<StringVarWrapper>();
    DCHECK_WARN(ib, "Non-TextWrapper instance has no wrapped attribute. This should be an error unless you're inspecting it.");
    return ib ? ib->wrapped() : nullptr;
}

void StringType::set(const sp<StringVar>& self, sp<StringVar> value)
{
    const sp<StringVarWrapper>& svw = self.ensureInstance<StringVarWrapper>("Setting on an instance of non StringVarWrapper");
    svw->set(std::move(value));
}

void StringType::set(const sp<StringVar>& self, String value)
{
    const sp<StringVarWrapper>& svi = self.ensureInstance<StringVarWrapper>("Setting on an instance of non StringVarWrapper");
    svi->set(sp<StringVar>::make<StringVarImpl>(std::move(value)));
}

sp<StringVar> StringType::ifElse(sp<StringVar> self, sp<Boolean> condition, sp<StringVar> negative)
{
    return sp<StringVar>::make<VariableTernary<StringView>>(std::move(condition), std::move(self), std::move(negative));
}

sp<StringVar> StringType::freeze(const sp<StringVar>& self)
{
    return create(self->val().data());
}

sp<StringVar> StringType::format(const String& format, const Scope& kwargs)
{
    static std::regex VAR_PATTERN("\\{(\\w+)(\\:[^}]+)?\\}");
    Vector<sp<StringVar>> strList;
    format.search(VAR_PATTERN, [&strList, &kwargs] (const std::smatch& matched) {
        const std::string name = matched[1].str();
        if(name.at(1) == '{') {
            strList.push_back(sp<StringVar>::make<StringVarImpl>(name));
            return true;
        }

        const std::string m2 = matched[2].str();
        const Optional<Box> value = kwargs.getObject(name);
        CHECK(value, "Unable to get keyword name \"%s\"", name.c_str());
        sp<StringVar> formatted = toStringVar<StringView, float, int32_t>(value.value(), m2.c_str());
        CHECK(formatted, "Unable to format key \"%s\"", name.c_str());
        strList.push_back(std::move(formatted));
        return true;
    }, [&strList] (const String& unmatched) {
        strList.push_back(sp<StringVar>::make<StringVarImpl>(unmatched));
        return true;
    });

    return sp<StringVar>::make<StringVarList>(std::move(strList));
}

sp<StringVar> StringType::dye(sp<StringVar> self, sp<Boolean> condition, String message)
{
    return sp<StringVar>::make<VariableDyed<StringView>>(std::move(self), std::move(condition), std::move(message));
}

StringType::DICTIONARY::DICTIONARY(BeanFactory& factory, const String& expr)
    : _value(factory.ensureBuilder<String>(expr))
{
}

sp<StringVar> StringType::DICTIONARY::build(const Scope& args)
{
    return StringType::create(*_value->build(args));
}

StringType::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _value(factory.ensureBuilder<String>(manifest, constants::VALUE)) {
}

sp<StringVar> StringType::BUILDER::build(const Scope& args)
{
    return StringType::create(*_value->build(args));
}

}
