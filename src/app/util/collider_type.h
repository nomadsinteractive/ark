#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/base/raycast_manifold.h"
#include "app/components/rigidbody.h"

namespace ark {

//[[script::bindings::class("Collider")]]
class ARK_API ColliderType {
public:
//  [[script::bindings::classmethod]]
    static sp<Rigidbody> createBody(const sp<Collider>& self, Rigidbody::BodyType bodyType, sp<Shape> shape, sp<Vec3> position = nullptr, sp<Vec4> rotation = nullptr, sp<CollisionFilter> collisionFilter = nullptr, sp<Boolean> discarded = nullptr);
//  [[script::bindings::classmethod]]
    static sp<Shape> createShape(const sp<Collider>& self, const NamedHash& type, sp<Vec3> size = nullptr, sp<Vec3> origin = nullptr);
//  [[script::bindings::classmethod]]
    static Vector<RayCastManifold> rayCast(const sp<Collider>& self, const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr);

};

}
