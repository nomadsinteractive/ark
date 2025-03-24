#pragma once

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
    PrimitiveModelFactory(sp<Mat4> transform = nullptr);

//  [[script::bindings::auto]]
    sp<Model> makeTriangle(Optional<Rect> texCoords = Optional<Rect>()) const;

//  [[script::bindings::auto]]
    sp<Model> makePlane(uint32_t cols, uint32_t rows, Optional<Rect> texCoords = Optional<Rect>()) const;

private:
    sp<Mat4> _transform;
};

}
