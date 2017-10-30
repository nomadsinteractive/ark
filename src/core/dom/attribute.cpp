#include "core/dom/attribute.h"

namespace ark {

Attribute::Attribute()
{
}

Attribute::Attribute(const String& name)
    : _name(name)
{
}

Attribute::Attribute(const String& name, const String& value)
    : _name(name), _value(value)
{
}

const String& Attribute::name() const
{
    return _name;
}

const String& Attribute::value() const
{
    return _value;
}

void Attribute::setValue(const String& value)
{
    _value = value;
}

}
