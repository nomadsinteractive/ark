#pragma once

#include "core/base/api.h"
#include "core/base/timestamp.h"
#include "core/inf/builder.h"
#include "core/inf/wirable.h"
#include "core/types/box.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"

namespace ark {

class ARK_API RigidBody : public Wirable {
public:
    class ARK_API Callback {
    public:
        void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold);
        void onEndContact(const RigidBody& rigidBody);

        void onBeginContact(const RigidBody& self, const RigidBody& rigidBody, const CollisionManifold& manifold);
        void onEndContact(const RigidBody& self, const RigidBody& rigidBody);

        bool hasCallback() const;

    private:
        sp<CollisionCallback> _collision_callback;

        friend class RigidBody;
    };

public:
    RigidBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotation, Box impl, SafeVar<Boolean> discarded);
    ~RigidBody() override;
    DISALLOW_COPY_AND_ASSIGN(RigidBody);

//  [[script::bindings::auto]]
    virtual void dispose() = 0;

    std::vector<std::pair<TypeId, Box>> onWire(const Traits& components) override;

//  [[script::bindings::property]]
    uintptr_t id() const;
    const sp<RigidBodyRef>& ref() const;
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
    const sp<Rotation>& rotation() const;

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

    const sp<Callback>& callback() const;

protected:
    sp<RigidBodyRef> _ref;

    Collider::BodyType _type;
    uint32_t _meta_id;
    sp<Shape> _shape;
    SafeVar<Vec3> _position;
    sp<Rotation> _rotation;

    Box _impl;
    SafeVar<Boolean> _discarded;

    sp<Callback> _callback;
    sp<CollisionFilter> _collision_filter;
};

}
