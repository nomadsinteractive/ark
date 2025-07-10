#pragma once

#include "core/base/api.h"
#include "core/base/string.h"

namespace ark {

class ARK_API DOMAttribute {
public:
    DOMAttribute() = default;
    DOMAttribute(String name);
//  [[script::bindings::auto]]
    DOMAttribute(String name, String value);

//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    const String& value() const;
//  [[script::bindings::property]]
    void setValue(String value);

private:
    String _name;
    String _value;
};

}
