#include "core/dom/dom_attribute.h"

namespace ark {

DOMAttribute::DOMAttribute(String name)
    : _name(std::move(name))
{
}

DOMAttribute::DOMAttribute(String name, String value)
    : _name(std::move(name)), _value(std::move(value))
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

void DOMAttribute::setValue(String value)
{
    _value = std::move(value);
}

}
