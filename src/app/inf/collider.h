#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/base/raycast_manifold.h"

namespace ark {

class ARK_API Collider {
public:
//  [[script::bindings::enumeration]]
    enum BodyShape {
        BODY_SHAPE_NONE = 0,
        BODY_SHAPE_AABB = -1,
        BODY_SHAPE_BALL = -2,
        BODY_SHAPE_BOX = -3,
        BODY_SHAPE_CAPSULE = -4
    };

//  [[script::bindings::enumeration]]
    enum BodyType {
        BODY_TYPE_KINEMATIC = 1,
        BODY_TYPE_DYNAMIC = 2,
        BODY_TYPE_STATIC = 4,
        BODY_TYPE_RIGID = 7,

        BODY_TYPE_SENSOR = 8,
        BODY_TYPE_ALL = 15,

        BODY_FLAG_MANUAL_ROTATION = 16
    };

//  [[script::bindings::constant]]
    static const String SHAPE_TYPE_NONE;
//  [[script::bindings::constant]]
    static const String SHAPE_TYPE_AABB;
//  [[script::bindings::constant]]
    static const String SHAPE_TYPE_BALL;
//  [[script::bindings::constant]]
    static const String SHAPE_TYPE_BOX;
//  [[script::bindings::constant]]
    static const String SHAPE_TYPE_CAPSULE;

public:
    virtual ~Collider() = default;

//  [[script::bindings::auto]]
    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shapeId, const sp<Vec3>& position, const sp<Size>& size = nullptr, const sp<Rotation>& rotate = nullptr, sp<Boolean> disposed = nullptr) = 0;
//  [[script::bindings::auto]]
    virtual std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;

};

}
