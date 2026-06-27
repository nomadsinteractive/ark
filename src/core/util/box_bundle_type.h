#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

namespace ark {

//[[script::bindings::class("BoxBundle")]]
class ARK_API BoxBundleType final {
public:

//  [[script::bindings::map(get)]]
    static Box get(BoxBundle& self, const String& name);

//  [[script::bindings::classmethod]]
    static Vector<String> keys(const BoxBundle& self);
//  [[script::bindings::classmethod]]
    static Vector<Box> values(const BoxBundle& self);
};

}
