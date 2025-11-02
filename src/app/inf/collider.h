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

//  [[script::bindings::classmethod]]
    static sp<Rigidbody> createBody(const sp<Collider>& self, Rigidbody::BodyType bodyType, sp<Shape> shape = nullptr, sp<Vec3> position = nullptr, sp<Vec4> rotation = nullptr, sp<CollisionFilter> collisionFilter = nullptr, sp<Boolean> discarded = nullptr);
    virtual Rigidbody::Impl createBody(Rigidbody::BodyType bodyType, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded) = 0;

//  [[script::bindings::auto]]
    virtual sp<Shape> createShape(const NamedHash& type, Optional<V3> scale = {}, const V3& origin = V3(0)) = 0;
//  [[script::bindings::auto]]
    virtual Vector<RayCastManifold> rayCast(V3 from, V3 to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;
//  [[script::bindings::auto]]
    virtual sp<Constraint> createFixedConstraint(Rigidbody& rigidbodyA, Rigidbody& rigidbodyB) = 0;

};

}
