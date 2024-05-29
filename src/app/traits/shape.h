#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

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
    Shape(int32_t id = Shape::SHAPE_ID_NONE, sp<Size> size = nullptr);

//  [[script::bindings::property]]
    int32_t id() const;
//  [[script::bindings::property]]
    void setId(int32_t id);

//  [[script::bindings::property]]
    const sp<Size>& size() const;
//  [[script::bindings::property]]
    void setSize(sp<Size> size);

private:
    int32_t _id;
    sp<Size> _size;
};

}
