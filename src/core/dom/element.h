#ifndef ARK_CORE_DOM_ELEMENT_H_
#define ARK_CORE_DOM_ELEMENT_H_

#include <map>

#include "core/base/api.h"
#include "core/dom/attribute.h"
#include "core/collection/iterable.h"
#include "core/collection/list.h"

namespace ark {

class ARK_API Element : public Attribute, public Iterable<std::map<String, attribute>> {
public:
    typedef std::map<String, attribute>::iterator Iterator;

    Element();
    Element(const String& name);
//  [[script::bindings::auto]]
    Element(const String& name, const String& value);
/*
//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const String& value() const;
//  [[script::bindings::property]]
    void setValue(const String& value);
*/
//  [[script::bindings::property]]
    const List<attribute>& attributes() const;

//  [[script::bindings::auto]]
    const sp<Attribute>& getAttribute(const String& name) const;
//  [[script::bindings::auto]]
    void setAttribute(const String& name, const String& value);
//  [[script::bindings::auto]]
    void addAttribute(const sp<Attribute>& attr);

private:
    std::map<String, attribute> _attributes_map;
    List<attribute> _attributes;
};

}

#endif
