#ifndef ARK_CORE_DOM_DOCUMENT_H_
#define ARK_CORE_DOM_DOCUMENT_H_

#include <map>

#include "core/base/api.h"
#include "core/collection/list.h"
#include "core/dom/dom_element.h"

namespace ark {

class ARK_API Document : public DOMElement {
public:
//  [[script::bindings::auto]]
    Document(const String& name);
/*
//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const String& value() const;
//  [[script::bindings::property]]
    void setValue(const String& value);

//  [[script::bindings::property]]
    const List<attribute>& attributes() const;
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
    void addChild(const sp<Document>& doc);

//  [[script::bindings::property]]
    const List<document>& children() const;
    List<document>& children(const String& name);

//  [[script::bindings::auto]]
    void clear();

private:
    std::map<String, List<document>> _children_by_name;
    List<document> _children;
};

}

#endif
