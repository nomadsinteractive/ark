#ifndef ARK_CORE_DOM_ATTRIBUTE_H_
#define ARK_CORE_DOM_ATTRIBUTE_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/base/string.h"

namespace ark {

class ARK_API Attribute {
public:
    Attribute();
    Attribute(const String& name);
//  [[script::bindings::auto]]
    Attribute(const String& name, const String& value);

//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const String& value() const;
//  [[script::bindings::property]]
    void setValue(const String& value);

private:
    String _name;
    String _value;
};

}

#endif
