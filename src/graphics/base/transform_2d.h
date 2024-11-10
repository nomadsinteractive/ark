#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "graphics/impl/transform/transform_impl.h"

namespace ark {

//[[script::bindings::extends(Transform)]]
class ARK_API Transform2D final : public TransformImpl, Implements<Transform2D, TransformImpl, Transform, Mat4> {
public:
//  [[script::bindings::auto]]
    Transform2D(sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr);

};

}
