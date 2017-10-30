#include "core/dom/element.h"

#include "core/dom/attribute.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

Element::Element()
    : Attribute(), Iterable(_attributes_map)
{
}

Element::Element(const String& name)
    : Attribute(name), Iterable(_attributes_map)
{
}

Element::Element(const String& name, const String& value)
    : Attribute(name, value), Iterable(_attributes_map)
{
}

const List<attribute>& Element::attributes() const
{
    return _attributes;
}

const sp<Attribute>& Element::getAttribute(const String& name) const
{
    auto iter = _attributes_map.find(name);
    if(iter != _attributes_map.end())
        return iter->second;
    return attribute::null();
}

void Element::setAttribute(const String& name, const String& value)
{
    auto iter = _attributes_map.find(name);
    if(iter != _attributes_map.end())
    {
        iter->second->setValue(value);
        return;
    }
    addAttribute(attribute::make(name, value));
}

void Element::addAttribute(const sp<Attribute>& attr)
{
    _attributes_map[attr->name()] = attr;
    _attributes.push_back(attr);
}

}
