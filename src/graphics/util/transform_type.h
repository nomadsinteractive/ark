#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/json.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Transform")]]
class ARK_API TransformType {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_NONE,
        TYPE_LINEAR_2D,
        TYPE_LINEAR_3D,
        TYPE_DELEGATED
    };

//  [[script::bindings::constructor]]
    static sp<Transform> create(sp<Vec4> rotation = nullptr, sp<Vec3> scale = nullptr, sp<Vec3> translation = nullptr, TransformType::Type type = TransformType::TYPE_NONE);

//  [[script::bindings::property]]
    static sp<Vec3> translation(const sp<Transform>& self);
//  [[script::bindings::property]]
    static void setTranslation(const sp<Transform>& self, sp<Vec3> translation);

//  [[script::bindings::property]]
    static sp<Vec4> rotation(const sp<Transform>& self);
//  [[script::bindings::property]]
    static void setRotation(const sp<Transform>& self, sp<Vec4> rotation);

//  [[script::bindings::property]]
    static sp<Vec3> scale(const sp<Transform>& self);
//  [[script::bindings::property]]
    static void setScale(const sp<Transform>& self, sp<Vec3> scale);

//  [[script::bindings::auto]]
    static void reset(const sp<Transform>& self, sp<Mat4> matrix);
};

}