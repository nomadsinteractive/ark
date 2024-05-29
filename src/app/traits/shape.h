#pragma once

#include "core/forwarding.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Shape {
public:
//  [[script::bindings::enumeration]]
    enum ShapeId {
        SHAPE_ID_NONE = 0,
        SHAPE_ID_AABB = -1,
        SHAPE_ID_BALL = -2,
        SHAPE_ID_BOX = -3,
        SHAPE_ID_CAPSULE = -4
    };

public:
//  [[script::bindings::auto]]
    Shape(int32_t id = Shape::SHAPE_ID_NONE, sp<Vec3> size = nullptr);

//  [[script::bindings::property]]
    int32_t id() const;
//  [[script::bindings::property]]
    void setId(int32_t id);

    const SafeVar<Vec3>& size() const;
    void setSize(SafeVar<Vec3> size);

private:
    int32_t _id;
    SafeVar<Vec3> _size;
};

}
