#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/wirable.h"
#include "core/types/box.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"

namespace ark {

class ARK_API RigidBody : public Wirable {
public:
    RigidBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> quaternion, Box impl, sp<Boolean> discarded);
    RigidBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> quaternion, Box impl, sp<Boolean> discarded, sp<Ref> ref);
    ~RigidBody() override;
    DISALLOW_COPY_AND_ASSIGN(RigidBody);

//  [[script::bindings::auto]]
    virtual void dispose();

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[script::bindings::property]]
    const sp<Ref>& id() const;
//  [[script::bindings::property]]
    Collider::BodyType type() const;
//  [[script::bindings::property]]
    Collider::BodyType rigidType() const;
//  [[script::bindings::property]]
    uint32_t metaId() const;

//  [[script::bindings::property]]
    const sp<Shape>& shape() const;
//  [[script::bindings::property]]
    const SafeVar<Vec3>& position() const;
//  [[script::bindings::property]]
    const SafeVar<Vec4>& quaternion() const;

//  [[script::bindings::property]]
    const SafeVar<Boolean>& discarded() const;

//  [[script::bindings::property]]
    const sp<CollisionCallback>& collisionCallback() const;
//  [[script::bindings::property]]
    void setCollisionCallback(sp<CollisionCallback> collisionCallback);

//  [[script::bindings::property]]
    const sp<CollisionFilter>& collisionFilter() const;
//  [[script::bindings::property]]
    void setCollisionFilter(sp<CollisionFilter> collisionFilter);

    void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) const;
    void onEndContact(const RigidBody& rigidBody) const;

    void onBeginContact(const RigidBody& self, const RigidBody& rigidBody, const CollisionManifold& manifold) const;
    void onEndContact(const RigidBody& self, const RigidBody& rigidBody) const;

    sp<RigidBody> makeShadow() const;

protected:
    sp<Ref> _ref;

    Collider::BodyType _type;
    uint32_t _meta_id;
    sp<Shape> _shape;
    SafeVar<Vec3> _position;
    SafeVar<Vec4> _quaternion;

    Box _impl;
    SafeVar<Boolean> _discarded;

    sp<CollisionCallback> _collision_callback;
    sp<CollisionFilter> _collision_filter;
};

}
