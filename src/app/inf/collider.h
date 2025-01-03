#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
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
        BODY_TYPE_KINEMATIC,
        BODY_TYPE_DYNAMIC,
        BODY_TYPE_STATIC,
        BODY_TYPE_RIGID = 3,
        BODY_TYPE_SENSOR
    };

public:
    virtual ~Collider() = default;

//  [[script::bindings::auto]]
    virtual sp<Rigidbody> createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position = nullptr, sp<Vec4> rotation = nullptr, sp<Boolean> discarded = nullptr) = 0;
//  [[script::bindings::auto]]
    virtual sp<Shape> createShape(const NamedHash& type, sp<Vec3> size = nullptr, sp<Vec3> origin = nullptr) = 0;
//  [[script::bindings::auto]]
    virtual std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;

};

}
