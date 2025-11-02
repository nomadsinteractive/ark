#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/base/constraint.h"
#include "app/base/raycast_manifold.h"
#include "app/components/rigidbody.h"

namespace ark {

class ARK_API Collider {
public:
    virtual ~Collider() = default;

    virtual Rigidbody::Impl createBody(Rigidbody::BodyType bodyType, sp<Shape> shape, sp<Vec3> position = nullptr, sp<Vec4> rotation = nullptr, sp<CollisionFilter> collisionFilter = nullptr, sp<Boolean> discarded = nullptr) = 0;
    virtual sp<Shape> createShape(const NamedHash& type, Optional<V3> scale = {}, const V3& origin = V3(0)) = 0;
    virtual sp<Constraint> createConstraint(Constraint::Type type, Rigidbody& rigidbodyA, Rigidbody& rigidbodyB, const V3& contactPoint) = 0;
    virtual Vector<RayCastManifold> rayCast(V3 from, V3 to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;

};

}
