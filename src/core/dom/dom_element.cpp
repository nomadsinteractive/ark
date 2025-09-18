#include "core/dom/dom_element.h"

#include "core/dom/dom_attribute.h"
#include "core/types/shared_ptr.h"

namespace ark {

DOMElement::DOMElement(String name, String value, Table<String, attribute> attributes)
    : DOMAttribute(std::move(name), std::move(value)), _attributes(std::move(attributes))
{
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

}
