#pragma once

#include "core/base/api.h"
#include "graphics/base/transform.h"

namespace ark {

//[[script::bindings::extends(Transform)]]
class ARK_API Transform2D final : public Transform, Implements<Transform2D, Transform, Mat4> {
public:
//  [[script::bindings::auto]]
    Transform2D(sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr);

};

} // ark
