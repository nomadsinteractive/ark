#include "core/dom/dom_element.h"

#include "core/dom/dom_attribute.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

DOMElement::DOMElement()
    : DOMAttribute(), Iterable(_attributes_map)
{
}

DOMElement::DOMElement(const String& name)
    : DOMAttribute(name), Iterable(_attributes_map)
{
}

DOMElement::DOMElement(const String& name, const String& value)
    : DOMAttribute(name, value), Iterable(_attributes_map)
{
}

const List<attribute>& DOMElement::attributes() const
{
    return _attributes;
}

const sp<DOMAttribute>& DOMElement::getAttribute(const String& name) const
{
    auto iter = _attributes_map.find(name);
    if(iter != _attributes_map.end())
        return iter->second;
    return attribute::null();
}

void DOMElement::setAttribute(const String& name, const String& value)
{
    auto iter = _attributes_map.find(name);
    if(iter != _attributes_map.end())
    {
        iter->second->setValue(value);
        return;
    }
    addAttribute(attribute::make(name, value));
}

void DOMElement::addAttribute(const sp<DOMAttribute>& attr)
{
    _attributes_map[attr->name()] = attr;
    _attributes.push_back(attr);
}

}
