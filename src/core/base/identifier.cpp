#include "core/base/identifier.h"

#include "core/util/strings.h"
#include "core/util/string_convert.h"

namespace ark {

namespace {

bool parseAndValidate(const String& s, String& package, String& value, bool strictMode)
{
    if(auto [packageCut, valueOpt] = s.cut(':'); valueOpt)
    {
        package = std::move(packageCut);
        value = std::move(valueOpt.value());
    }
    else
    {
        package = "";
        value = std::move(packageCut);
    }
    if(strictMode)
        return Strings::isVariableName(package) && Strings::isVariableName(value);
    return Strings::isVariableName(package);
}

bool parseTypeAndValue(const String& s, String& type, String& value)
{
    if(const String::size_type pos = s.find('('); pos > 0 && pos != String::npos && s.back() == ')')
    {
        value = s.substr(0, pos);
        type = s.substr(pos + 1, s.length() - 1);
        return true;
    }
    return false;
}

Optional<Identifier> tryParse(Identifier::Type tokenType, const String& s, bool strict)
{
    String package;
    String value;

    if(auto [packageCut, valueOpt] = s.cut(':'); valueOpt)
    {
        package = std::move(packageCut);
        value = std::move(valueOpt.value());
    }
    else
    {
        package = "";
        value = std::move(packageCut);
    }

    bool isOptional = false;
    if(!value.empty() && value.startsWith("?"))
    {
        value = value.substr(1);
        isOptional = true;
    }

    if(const bool isPkgVarName = Strings::isVariableName(package); strict ? isPkgVarName && Strings::isVariableName(value) : isPkgVarName)
        return {Identifier(tokenType, std::move(package), std::move(value), {}, isOptional)};

    return {};
}

}

Identifier::Identifier(Type type, String package, String value, String valueType, bool isOptional)
    : _type(type), _package(std::move(package)), _value(std::move(value)), _value_type(std::move(valueType)), _is_optional(isOptional)
{
}

Identifier Identifier::parse(const String& s, Identifier::Type idType, bool strictMode)
{
    CHECK(s, "Illegal identifier: empty string");

    const Type headTypeToken = static_cast<Type>(s.at(0));
    const Type idTypeToken = idType == ID_TYPE_AUTO ? headTypeToken : idType;

    if(idTypeToken == ID_TYPE_REFERENCE)
        if(Optional<Identifier> idOpt = tryParse(ID_TYPE_REFERENCE, headTypeToken == ID_TYPE_REFERENCE ? s.substr(1) : s, strictMode))
            return std::move(idOpt.value());

    if(idTypeToken == ID_TYPE_ARGUMENT)
        if(Optional<Identifier> idOpt = tryParse(ID_TYPE_ARGUMENT, headTypeToken == ID_TYPE_ARGUMENT ? s.substr(1) : s, strictMode))
            return std::move(idOpt.value());

    if(idTypeToken == ID_TYPE_EXPRESSION)
    {
        CHECK(s.back() == '}', "Illegal identifier, expression not in braces: %s", s.c_str());
        return Identifier(ID_TYPE_EXPRESSION, {}, s.substr(1, s.length() - 1).strip());
    }

    String package, value, valueType;
    if(idTypeToken == ID_TYPE_VALUE_AND_TYPE || parseTypeAndValue(headTypeToken == ID_TYPE_VALUE_AND_TYPE ? s.substr(1) : s, value, valueType))
        return Identifier(ID_TYPE_VALUE_AND_TYPE, package, value, valueType);

    if(Optional<Identifier> idOpt = tryParse(idType == ID_TYPE_AUTO ? ID_TYPE_VALUE : idType, s, false))
        return std::move(idOpt.value());

    return Identifier(ID_TYPE_VALUE, "", s);
}

Identifier Identifier::parseRef(const String& s, bool strictMode)
{
    CHECK(s, "Illegal identifier: empty string");
    String package, ref;
    const bool idValid = parseAndValidate(s, package, ref, strictMode);
    CHECK_WARN(idValid, "Unvaild refid \"%s\"", s.c_str());
    return idValid ? Identifier(ID_TYPE_REFERENCE, package, ref) : Identifier(ID_TYPE_REFERENCE, "", s);
}

Identifier::operator bool() const
{
    return !_value.empty();
}

Identifier::Type Identifier::type() const
{
    return _type;
}

const String& Identifier::package() const
{
    return _package;
}

const String& Identifier::arg() const
{
    return _type == ID_TYPE_ARGUMENT ? _value : String::null();
}

const String& Identifier::ref() const
{
    return _type == ID_TYPE_REFERENCE ? _value : String::null();
}

const String& Identifier::val() const
{
    return (_type == ID_TYPE_VALUE || _type == ID_TYPE_VALUE_AND_TYPE || _type == ID_TYPE_EXPRESSION) ? _value : String::null();
}

const String& Identifier::valType() const
{
    return _type == ID_TYPE_VALUE_AND_TYPE ? _value_type : String::null();
}

String Identifier::toString() const
{
    StringBuffer sb;
    if(_type == ID_TYPE_ARGUMENT)
        sb << '$';
    else if(_type == ID_TYPE_REFERENCE)
        sb << '@';
    else if(_type == ID_TYPE_EXPRESSION)
        sb << '{';
    if(_package)
        sb << _package << ':';
    sb << _value;
    if(_type == ID_TYPE_EXPRESSION)
        sb << '}';
    return sb.str();
}

bool Identifier::isRef() const
{
    return _type == ID_TYPE_REFERENCE;
}

bool Identifier::isArg() const
{
    return _type == ID_TYPE_ARGUMENT;
}

bool Identifier::isVal() const
{
    return _type == ID_TYPE_VALUE;
}

bool Identifier::isOptional() const
{
    return _is_optional;
}

Identifier Identifier::withouPackage() const
{
    Identifier wp = *this;
    wp._package = "";
    return wp;
}

template<> Identifier StringConvert::eval<Identifier>(const String& val)
{
    return Identifier::parse(val);
}

template<> String StringConvert::repr<Identifier>(const Identifier& val)
{
    return val.toString();
}

}
