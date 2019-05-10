#ifndef ARK_CORE_DOM_DOCUMENT_H_
#define ARK_CORE_DOM_DOCUMENT_H_

#include <map>

#include "core/base/api.h"
#include "core/dom/dom_element.h"

namespace ark {

class ARK_API DOMDocument : public DOMElement {
public:
//  [[script::bindings::auto]]
    DOMDocument(const String& name);
/*
//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const String& value() const;
//  [[script::bindings::property]]
    void setValue(const String& value);

//  [[script::bindings::property]]
    const std::vector<attribute>& attributes() const;
//  [[script::bindings::auto]]
    const sp<DOMAttribute>& getAttribute(const String& name) const;
//  [[script::bindings::auto]]
    void setAttribute(const String& name, const String& value);
//  [[script::bindings::auto]]
    void addAttribute(const sp<DOMAttribute>& attr);
*/

//  [[script::bindings::auto]]
    const document& getChild(const String& name) const;
//  [[script::bindings::auto]]
    const document& ensureChild(const String& name) const;
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
