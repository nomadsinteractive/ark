#ifndef ARK_APP_INF_COLLIDER_H_
#define ARK_APP_INF_COLLIDER_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Collider {
public:
//  [[script::bindings::enumeration]]
    enum BodyShape {
        BODY_SHAPE_AABB = -1,
        BODY_SHAPE_BALL = -2,
        BODY_SHAPE_BOX = -3
    };

//  [[script::bindings::enumeration]]
    enum BodyType {
        BODY_TYPE_DYNAMIC = 0,
        BODY_TYPE_KINEMATIC = 1,
        BODY_TYPE_STATIC = 2,
        BODY_TYPE_MASK = 3,

        BODY_FLAG_MANUAL_POSITION = 4,
        BODY_FLAG_MANUAL_ROTATION = 8
    };

public:
    virtual ~Collider() = default;

//  [[script::bindings::auto]]
    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size = nullptr, const sp<Rotate>& rotate = nullptr) = 0;

};

}

#endif
