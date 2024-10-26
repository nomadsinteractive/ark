#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/base/raycast_manifold.h"

namespace ark {

class ARK_API Collider {
public:
//  [[script::bindings::enumeration]]
    enum BodyType {
        BODY_TYPE_NONE = 0,
        BODY_TYPE_KINEMATIC = 1,
        BODY_TYPE_DYNAMIC = 2,
        BODY_TYPE_STATIC = 4,
        BODY_TYPE_RIGID = 7,

        BODY_TYPE_SENSOR = 8,
        BODY_TYPE_ALL = 15,
    };

public:
    virtual ~Collider() = default;

//  [[script::bindings::auto]]
    virtual sp<Rigidbody> createBody(Collider::BodyType type, sp<Shape> shape = nullptr, sp<Vec3> position = nullptr, sp<Rotation> rotate = nullptr, sp<Boolean> discarded = nullptr) = 0;
//  [[script::bindings::auto]]
    virtual std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;

};

}
