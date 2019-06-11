#include "core/base/identifier.h"

#include "core/util/strings.h"
#include "core/types/null.h"

namespace ark {

Identifier::Identifier(IdType type, const String& package, const String& value)
    : _type(type), _package(package), _value(value)
{
}

Identifier Identifier::parse(const String& s, Format format)
{
    DCHECK(s, "Illegal identifier: empty string");
    String package, value;
    IdType idType = static_cast<IdType>(s.at(0));

    if(idType == ID_TYPE_REFERENCE && parseAndVaildate(s.substr(1), package, value, format))
        return Identifier(ID_TYPE_REFERENCE, package, value);

    if(idType == ID_TYPE_ARGUMENT && parseAndVaildate(s.substr(1), package, value, format))
        return Identifier(ID_TYPE_ARGUMENT, package, value);

    return Identifier(ID_TYPE_VALUE, package, s);
}

Identifier Identifier::parseRef(const String& s)
{
    DCHECK(s, "Illegal identifier: empty string");
    String package, ref;
    Strings::cut(s, package, ref, ':');
    return Identifier(ID_TYPE_REFERENCE, package, ref);
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
    return _type == ID_TYPE_VALUE ? _value : String::null();
}

String Identifier::toString() const
{
    StringBuffer sb;
    if(_type == ID_TYPE_ARGUMENT)
        sb << '$';
    else if(_type == ID_TYPE_REFERENCE)
        sb << '@';
    if(_package)
        sb << _package << ':';
    sb << _value;
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

bool Identifier::parseAndVaildate(const String& s, String& package, String& value, Format format)
{
    Strings::cut(s, package, value, ':');
    if(FORMAT_NAMESPACE_STRICT == format)
        return Strings::isVariableName(package) && Strings::isVariableName(value);
    return true;
}

}
