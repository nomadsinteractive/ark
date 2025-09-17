#pragma once

#include "core/base/api.h"
#include "core/dom/dom_attribute.h"
#include "core/collection/table.h"
#include "core/types/implements.h"

namespace ark {

//[[script::bindings::extends(DOMAttribute)]]
class ARK_API DOMElement : public DOMAttribute, public Implements<DOMElement, DOMAttribute> {
public:
    enum ElementType {
        ELEMENT_TYPE_ELEMENT,
        ELEMENT_TYPE_TEXT,
        ELEMENT_TYPE_COMMENT,
        ELEMENT_TYPE_UNKNOWN
    };
public:
    DOMElement();
    DOMElement(String name, DOMElement::ElementType type = DOMElement::ELEMENT_TYPE_ELEMENT);
//  [[script::bindings::auto]]
    DOMElement(String name, String value, DOMElement::ElementType type = DOMElement::ELEMENT_TYPE_ELEMENT);

//  [[script::bindings::property]]
    DOMElement::ElementType type() const;
//  [[script::bindings::property]]
    const Vector<sp<DOMAttribute>>& attributes() const;

//  [[script::bindings::auto]]
    sp<DOMAttribute> getAttribute(const String& name) const;
//  [[script::bindings::auto]]
    void setAttribute(const String& name, String value);
//  [[script::bindings::auto]]
    void addAttribute(sp<DOMAttribute> attr);

private:
    ElementType _type;
    Table<String, attribute> _attributes;
};

}
