#include "core/base/identifier.h"

#include "core/util/strings.h"
#include "core/types/null.h"

namespace ark {

Identifier::Identifier(IdType type, const String& package, const String& value, const String& queries)
    : _type(type), _package(package), _value(value)
{
    if(queries)
        parseQueries(queries);
}

Identifier Identifier::parse(const String& s, Format format)
{
    DCHECK(s, "Illegal identifier: empty string");
    String package, value, queries;
    IdType idType = static_cast<IdType>(s.at(0));

    if(idType == ID_TYPE_REFERENCE && parseAndVaildate(s.substr(1), package, value, queries, format))
        return Identifier(ID_TYPE_REFERENCE, package, value, queries);

    if(idType == ID_TYPE_ARGUMENT && parseAndVaildate(s.substr(1), package, value, queries, format))
        return Identifier(ID_TYPE_ARGUMENT, package, value, queries);

    parseAndVaildate(s, package, value, queries, FORMAT_NAMESPACE);
    return Identifier(ID_TYPE_VALUE, package, value, queries);
}

Identifier Identifier::parseRef(const String& s)
{
    DCHECK(s, "Illegal identifier: empty string");
    String package, ref, queries;
    parseAndVaildate(s, package, ref, queries, FORMAT_NAMESPACE_STRICT);
    return Identifier(ID_TYPE_REFERENCE, package, ref, queries);
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

bool Identifier::parseAndVaildate(const String& s, String& package, String& value, String& queries, Format format)
{
    String packageAndName = s;
    Strings::cut(s, packageAndName, queries, '?', false);
    Strings::cut(packageAndName, package, value, ':');
    if(FORMAT_NAMESPACE_STRICT == format)
        return Strings::isVariableName(package) && Strings::isVariableName(value);
    return true;
}

void Identifier::parseQueries(const String& queries)
{
    for(const String& i : queries.split('&'))
    {
        String name, value;
        Strings::cut(i, name, value, '=');
        _queries.push_back(name.strip(), value.strip());
    }
}

}
