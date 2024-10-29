#pragma once

#include "core/base/api.h"
#include "graphics/base/transform.h"

namespace ark {

//[[script::bindings::extends(Transform)]]
class ARK_API Transform3D final : public Transform, Implements<Transform3D, Transform, Mat4> {
public:
//  [[script::bindings::auto]]
    Transform3D(sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr);

};

}
