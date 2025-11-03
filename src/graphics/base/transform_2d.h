#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/impl/transform/transform_impl.h"

namespace ark {

//[[script::bindings::extends(Transform)]]
class ARK_API Transform2D final : public TransformImpl, Implements<Transform2D, TransformImpl, Transform, Mat4> {
public:
//  [[script::bindings::constructor(Transform)]]
    Transform2D(sp<Numeric> rotation = nullptr, sp<Vec2> scale = nullptr, sp<Vec2> translation = nullptr);
};

}
