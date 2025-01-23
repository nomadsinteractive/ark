#include "core/base/identifier.h"

#include "core/util/strings.h"

namespace ark {

namespace {

bool parseAndValidate(const String& s, String& package, String& value, String& queries, bool strictMode)
{
    auto [packageAndName, queriesOpt] = s.cut('?');
    if(auto [packageCut, valueOpt] = packageAndName.cut(':'); valueOpt)
    {
        package = std::move(packageCut);
        value = std::move(valueOpt.value());
    }
    else
    {
        package = "";
        value = std::move(packageCut);
    }
    if(queriesOpt)
        queries = std::move(queriesOpt.value());
    if(strictMode)
        return Strings::isVariableName(package) && Strings::isVariableName(value);
    return Strings::isVariableName(package);
}

bool parseTypeAndValue(const String& s, String& type, String& value)
{
    const String::size_type pos = s.find('(');
    if(pos > 0 && pos != String::npos && s.back() == ')')
    {
        value = s.substr(0, pos);
        type = s.substr(pos + 1, s.length() - 1);
        return true;
    }
    return false;
}

}

Identifier::Identifier(Type type, String package, String value, String valueType)
    : _type(type), _package(std::move(package)), _value(std::move(value)), _value_type(std::move(valueType))
{
}

Identifier Identifier::parse(const String& s, Identifier::Type idType, bool strictMode)
{
    CHECK(s, "Illegal identifier: empty string");

    Identifier::Type headTypeToken = static_cast<Type>(s.at(0));
    Identifier::Type idTypeToken = idType == ID_TYPE_AUTO ? headTypeToken : idType;

    String package, value, valueType, queries;
    if(idTypeToken == ID_TYPE_REFERENCE && parseAndValidate(headTypeToken == ID_TYPE_REFERENCE ? s.substr(1) : s, package, value, queries, strictMode))
        return Identifier(ID_TYPE_REFERENCE, package, value, "");

    if(idTypeToken == ID_TYPE_ARGUMENT && parseAndValidate(headTypeToken == ID_TYPE_ARGUMENT ? s.substr(1) : s, package, value, queries, strictMode))
        return Identifier(ID_TYPE_ARGUMENT, package, value, "");

    if(idTypeToken == ID_TYPE_EXPRESSION)
    {
        CHECK(s.back() == '}', "Illegal identifier, expression not in braces: %s", s.c_str());
        return Identifier(ID_TYPE_EXPRESSION, package, s.substr(1, s.length() - 1).strip(), "");
    }

    if(idTypeToken == ID_TYPE_VALUE_AND_TYPE || parseTypeAndValue(headTypeToken == ID_TYPE_VALUE_AND_TYPE ? s.substr(1) : s, value, valueType))
        return Identifier(ID_TYPE_VALUE_AND_TYPE, package, value, valueType);

    return parseAndValidate(s, package, value, queries, false) ? Identifier(idType == ID_TYPE_AUTO ? ID_TYPE_VALUE : idType, package, value, "") : Identifier(ID_TYPE_VALUE, "", s, "");
}

Identifier Identifier::parseRef(const String& s, bool strictMode)
{
    DCHECK(s, "Illegal identifier: empty string");
    String package, ref, queries;
    bool idValid = parseAndValidate(s, package, ref, queries, strictMode);
    DCHECK_WARN(idValid, "Unvaild refid \"%s\"", s.c_str());
    return idValid ? Identifier(ID_TYPE_REFERENCE, package, ref, "") : Identifier(ID_TYPE_REFERENCE, "", s, "");
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

const Table<String, String>& Identifier::queries() const
{
    return _queries;
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

Identifier Identifier::withouPackage() const
{
    Identifier wp = *this;
    wp._package = "";
    return wp;
}

}
