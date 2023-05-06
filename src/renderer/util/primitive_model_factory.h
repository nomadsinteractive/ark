#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/optional.h"

#include "graphics/forwarding.h"
#include "graphics/base/rect.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API PrimitiveModelFactory {
public:
//  [[script::bindings::constructor]]
    PrimitiveModelFactory();

//  [[script::bindings::auto]]
    sp<Model> makeTriangle(Optional<Rect> texCoords = Optional<Rect>());

//  [[script::bindings::auto]]
    sp<Model> makePlane(uint32_t cols, uint32_t rows, Optional<Rect> texCoords = Optional<Rect>());

};

}
