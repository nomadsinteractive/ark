#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/wirable.h"
#include "core/types/box.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"
#include "core/types/safe_builder.h"

namespace ark {

class ARK_API Rigidbody : public Wirable {
public:
    Rigidbody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, Box impl, sp<Ref> ref, bool isShadow = false);
    ~Rigidbody() override;
    DISALLOW_COPY_AND_ASSIGN(Rigidbody);

//  [[script::bindings::auto]]
    virtual void discard();

    void onWire(const WiringContext& context, const Box& self) override;

//  [[script::bindings::property]]
    RefId id() const;
//  [[script::bindings::property]]
    const sp<Ref>& ref() const;
//  [[script::bindings::property]]
    Collider::BodyType type() const;

//  [[script::bindings::property]]
    const sp<Shape>& shape() const;
//  [[script::bindings::property]]
    const SafeVar<Vec3>& position() const;
//  [[script::bindings::property]]
    const SafeVar<Vec4>& rotation() const;

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

//  [[script::bindings::property]]
    Box tag() const;
//  [[script::bindings::property]]
    void setTag(Box tag);

    void onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) const;
    void onEndContact(const Rigidbody& rigidBody) const;

    void onBeginContact(const Rigidbody& self, const Rigidbody& rigidBody, const CollisionManifold& manifold) const;
    void onEndContact(const Rigidbody& self, const Rigidbody& rigidBody) const;

    sp<Rigidbody> makeShadow() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Rigidbody> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Rigidbody> build(const Scope& args) override;

    private:
        builder<Collider> _collider;
        Collider::BodyType _body_type;
        SafeBuilder<Shape> _shape;
        SafeBuilder<Vec3> _position;
        SafeBuilder<Vec4> _rotation;
        SafeBuilder<Boolean> _discarded;
    };

protected:
    sp<Ref> _ref;

    Collider::BodyType _type;
    sp<Shape> _shape;
    SafeVar<Vec3> _position;
    SafeVar<Vec4> _rotation;

    Box _impl;
    bool _is_shadow;

    sp<CollisionCallback> _collision_callback;
    sp<CollisionFilter> _collision_filter;
    sp<WithTag> _with_tag;
};

}
