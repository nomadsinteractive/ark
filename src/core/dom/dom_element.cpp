#include "core/dom/dom_element.h"

#include "core/dom/dom_attribute.h"
#include "core/types/shared_ptr.h"

namespace ark {

DOMElement::DOMElement()
    : DOMAttribute(), _type(ELEMENT_TYPE_ELEMENT)
{
}

DOMElement::DOMElement(String name, const ElementType type)
    : DOMAttribute(std::move(name)), _type(type)
{
}

DOMElement::DOMElement(String name, String value, const ElementType type)
    : DOMAttribute(std::move(name), std::move(value)), _type(type)
{
}

DOMElement::ElementType DOMElement::type() const
{
    return _type;
}

const Vector<sp<DOMAttribute>>& DOMElement::attributes() const
{
    return _attributes.values();
}

sp<DOMAttribute> DOMElement::getAttribute(const String& name) const
{
    const auto iter = _attributes.find(name);
    return iter != _attributes.end() ? iter->second : nullptr;
}

void DOMElement::setAttribute(const String& name, String value)
{
    if(const sp<DOMAttribute>& attr = getAttribute(name))
    {
        attr->setValue(std::move(value));
        return;
    }
    addAttribute(attribute::make(name, std::move(value)));
}

void DOMElement::addAttribute(sp<DOMAttribute> attr)
{
    const String& name = attr->name();
    _attributes.push_back(name, std::move(attr));
}

}
