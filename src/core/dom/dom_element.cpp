#include "core/dom/dom_element.h"

#include "core/dom/dom_attribute.h"
#include "core/types/null.h"
#include "core/types/shared_ptr.h"

namespace ark {

DOMElement::DOMElement()
    : DOMAttribute(), _type(ELEMENT_TYPE_ELEMENT)
{
}

DOMElement::DOMElement(const String& name, DOMElement::ElementType type)
    : DOMAttribute(name), _type(type)
{
}

DOMElement::DOMElement(const String& name, const String& value, DOMElement::ElementType type)
    : DOMAttribute(name, value), _type(type)
{
}

DOMElement::ElementType DOMElement::type() const
{
    return _type;
}

const std::vector<attribute>& DOMElement::attributes() const
{
    return _attributes.values();
}

const sp<DOMAttribute>& DOMElement::getAttribute(const String& name) const
{
    const auto iter = _attributes.find(name);
    if(iter != _attributes.end())
        return iter->second;
    return attribute::null();
}

void DOMElement::setAttribute(const String& name, const String& value)
{
    const sp<DOMAttribute>& attr = getAttribute(name);
    if(attr)
    {
        attr->setValue(value);
        return;
    }
    addAttribute(attribute::make(name, value));
}

void DOMElement::addAttribute(const sp<DOMAttribute>& attr)
{
    _attributes.push_back(attr->name(), attr);
}

}
