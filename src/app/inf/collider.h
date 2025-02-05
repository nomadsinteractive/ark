#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/base/raycast_manifold.h"
#include "app/components/rigidbody.h"

namespace ark {

class ARK_API Collider {
public:
    virtual ~Collider() = default;

    virtual Rigidbody::Impl createBody(Rigidbody::BodyType bodyType, sp<Shape> shape, sp<Vec3> position = nullptr, sp<Vec4> rotation = nullptr, sp<CollisionFilter> collisionFilter = nullptr, sp<Boolean> discarded = nullptr) = 0;
    virtual sp<Shape> createShape(const NamedHash& type, sp<Vec3> size = nullptr, sp<Vec3> origin = nullptr) = 0;
    virtual std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;

};

}
