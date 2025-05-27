#pragma once

#include "core/base/api.h"
#include "core/impl/builder/enum_builder.h"
#include "core/impl/builder/safe_builder.h"
#include "core/inf/wirable.h"
#include "core/types/box.h"
#include "core/types/safe_var.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Rigidbody final : public Wirable {
public:
//  [[script::bindings::enumeration]]
    enum BodyType {
        BODY_TYPE_NONE = 0,
        BODY_TYPE_KINEMATIC,
        BODY_TYPE_DYNAMIC,
        BODY_TYPE_STATIC,
        BODY_TYPE_RIGID = 3,
        BODY_TYPE_SENSOR = 1 << 2,
        BODY_TYPE_GHOST = 2 << 2
    };

    struct ARK_API Stub {
        Stub(sp<Ref> ref, BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter = nullptr);

        void onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) const;
        void onEndContact(const Rigidbody& rigidBody) const;

        sp<Ref> _ref;
        BodyType _type;
        sp<Shape> _shape;
        SafeVar<Vec3> _position;
        SafeVar<Vec4> _rotation;
        sp<CollisionFilter> _collision_filter;
        sp<CollisionCallback> _collision_callback;
        sp<Tags> _tags;
    };

    struct Impl {
        sp<Stub> _stub;
        sp<Collider> _collider;
        sp<RigidbodyController> _controller;
    };

public:
    Rigidbody(BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded, sp<RigidbodyController> controller, bool isShadow = false);
    Rigidbody(Impl impl, bool isShadow);
    ~Rigidbody() override;

    void onWire(const WiringContext& context, const Box& self) override;

//  [[script::bindings::property]]
    RefId id() const;
//  [[script::bindings::property]]
    Rigidbody::BodyType type() const;

//  [[script::bindings::property]]
    const sp<Shape>& shape() const;
//  [[script::bindings::property]]
    const SafeVar<Vec3>& position() const;
//  [[script::bindings::property]]
    const SafeVar<Vec4>& rotation() const;

//  [[script::bindings::auto]]
    void discard();
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

//  [[script::bindings::property]]
    const sp<RigidbodyController>& controller() const;

    void onBeginContact(const Rigidbody& rigidbody, const CollisionManifold& manifold) const;
    void onEndContact(const Rigidbody& rigidbody) const;

    sp<Rigidbody> makeShadow() const;

//  [[plugin::builder]]
    class BUILDER final : public Builder<Rigidbody> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Rigidbody> build(const Scope& args) override;

    private:
        builder<Collider> _collider;
        EnumBuilder<BodyType> _body_type;
        SafeBuilder<Shape> _shape;
        SafeBuilder<Vec3> _position;
        SafeBuilder<Vec4> _rotation;
        SafeBuilder<Boolean> _discarded;
        SafeBuilder<CollisionCallback> _collision_callback;
        SafeBuilder<CollisionFilter> _collision_filter;
    };

//  [[plugin::builder("with-rigidbody")]]
    class BUILDER_WIRABLE final : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        builder<Rigidbody> _rigidbody;
    };

protected:
    Impl _impl;
    bool _is_shadow;
};

}
