#pragma once

#include "core/base/api.h"
#include "core/dom/dom_attribute.h"
#include "core/collection/table.h"
#include "core/types/implements.h"

namespace ark {

//[[script::bindings::extends(DOMAttribute)]]
class ARK_API DOMElement : public DOMAttribute, public Implements<DOMElement, DOMAttribute> {
public:
    DOMElement(String name, String value, Table<String, attribute> attributes = {});

//  [[script::bindings::property]]
    const Vector<sp<DOMAttribute>>& attributes() const;

//  [[script::bindings::auto]]
    sp<DOMAttribute> getAttribute(const String& name) const;

private:
    ImmutableTable<String, attribute> _attributes;
};

}
