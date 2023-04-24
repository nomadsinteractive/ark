#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API PrimitiveModelFactory {
public:
//  [[script::bindings::constructor]]
    PrimitiveModelFactory();

//  [[script::bindings::auto]]
    sp<Model> makePlane(uint32_t subdivisons);

};

}
