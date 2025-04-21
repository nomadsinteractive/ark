#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/impl/transform/transform_impl.h"

namespace ark {

//[[script::bindings::extends(Transform)]]
class ARK_API Transform3D final : public TransformImpl, Implements<Transform3D, TransformImpl, Transform, Mat4> {
public:
    Transform3D(sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr);

//  [[script::bindings::constructor]]
    static sp<Transform> create(sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr);
};

}
