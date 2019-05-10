#ifndef ARK_CORE_DOM_ELEMENT_H_
#define ARK_CORE_DOM_ELEMENT_H_

#include <map>

#include "core/base/api.h"
#include "core/dom/dom_attribute.h"
#include "core/collection/iterable.h"
#include "core/collection/table.h"

namespace ark {

class ARK_API DOMElement : public DOMAttribute {
public:
    typedef std::map<String, attribute>::iterator Iterator;

    DOMElement();
    DOMElement(const String& name);
//  [[script::bindings::auto]]
    DOMElement(const String& name, const String& value);
/*
//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const String& value() const;
//  [[script::bindings::property]]
    void setValue(const String& value);
*/
//  [[script::bindings::property]]
    const std::vector<attribute>& attributes() const;

//  [[script::bindings::auto]]
    const sp<DOMAttribute>& getAttribute(const String& name) const;
//  [[script::bindings::auto]]
    void setAttribute(const String& name, const String& value);
//  [[script::bindings::auto]]
    void addAttribute(const sp<DOMAttribute>& attr);

private:
    Table<String, attribute> _attributes;
};

}

#endif
