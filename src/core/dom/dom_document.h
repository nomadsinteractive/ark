#ifndef ARK_CORE_DOM_DOCUMENT_H_
#define ARK_CORE_DOM_DOCUMENT_H_

#include <map>

#include "core/base/api.h"
#include "core/dom/dom_element.h"
#include "core/types/implements.h"

namespace ark {

//[[script::bindings::extends(DOMElement)]]
class ARK_API DOMDocument : public DOMElement, public Implements<DOMDocument, DOMElement, DOMAttribute> {
public:
//  [[script::bindings::auto]]
    DOMDocument(const String& name, const String& value = "", DOMElement::ElementType type = DOMElement::ELEMENT_TYPE_ELEMENT);

//  [[script::bindings::auto]]
    document getChild(const String& name) const;
//  [[script::bindings::auto]]
    document ensureChild(const String& name) const;
//  [[script::bindings::auto]]
    void addChild(const sp<DOMDocument>& doc);

//  [[script::bindings::property]]
    const std::vector<document>& children() const;
    const std::vector<document>& children(const String& name);

private:
    std::map<String, std::vector<document>> _children_by_name;
    std::vector<document> _children;
};

}

#endif
