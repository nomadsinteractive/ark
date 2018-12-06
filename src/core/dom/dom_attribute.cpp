#include "core/dom/dom_attribute.h"

namespace ark {

DOMAttribute::DOMAttribute()
{
}

DOMAttribute::DOMAttribute(const String& name)
    : _name(name)
{
}

DOMAttribute::DOMAttribute(const String& name, const String& value)
    : _name(name), _value(value)
{
}

const String& DOMAttribute::name() const
{
    return _name;
}

const String& DOMAttribute::value() const
{
    return _value;
}

void DOMAttribute::setValue(const String& value)
{
    _value = value;
}

}
