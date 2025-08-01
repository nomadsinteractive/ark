#pragma once

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/impl/builder/safe_builder.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"

#include "plugin/bullet/forwarding.h"
#include "plugin/bullet/api.h"

#include "btBulletDynamicsCommon.h"

namespace ark::plugin::bullet {

//[[script::bindings::extends(Collider)]]
//[[script::bindings::name("World")]]
class ARK_PLUGIN_BULLET_API ColliderBullet final : public Collider, Implements<ColliderBullet, Collider> {
public:
    class RigidbodyImporter {
    public:
        virtual ~RigidbodyImporter() = default;

        virtual void import(ColliderBullet& collider, const document& manifest) = 0;
    };

public:
    ColliderBullet(const V3& gravity, sp<ModelLoader> modelLoader);

    Rigidbody::Impl createBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position = nullptr, sp<Vec4> rotation = nullptr, sp<CollisionFilter> collisionFilter = nullptr, sp<Boolean> discarded = nullptr) override;
    sp<Shape> createShape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin) override;
    Vector<RayCastManifold> rayCast(V3 from, V3 to, const sp<CollisionFilter>& collisionFilter = nullptr) override;

//  [[script::bindings::auto]]
    void rayCastClosest(const V3& from, const V3& to, const sp<CollisionCallback>& callback, int32_t filterGroup = 1, int32_t filterMask = -1) const;

    btDiscreteDynamicsWorld* btDynamicWorld() const;

    const HashMap<TypeId, sp<CollisionShapeRef>>& collisionShapes() const;
    HashMap<TypeId, sp<CollisionShapeRef>>& collisionShapes();

    void markForDestroy(sp<CollisionObjectRef> collisionBody) const;

//  [[plugin::resource-loader]]
    class BUILDER_IMPL1 final : public Builder<ColliderBullet> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<ColliderBullet> build(const Scope& args) override;

    private:
        V3 _gravity;
        SafeBuilder<ModelLoader> _model_loader;
        Vector<std::pair<sp<Builder<RigidbodyImporter>>, document>> _importers;

        sp<ResourceLoaderContext> _resource_loader_context;
    };

//  [[plugin::resource-loader("btWorld")]]
    class BUILDER_IMPL2 final : public Builder<Collider> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Collider> build(const Scope& args) override;

    private:
        BUILDER_IMPL1 _impl;
    };

private:
    struct Stub;

    struct ContactInfo {
        Set<sp<CollisionObjectRef>> _last_tick;
        Set<sp<CollisionObjectRef>> _current_tick;
    };

    static void myInternalPreTickCallback(btDynamicsWorld* dynamicsWorld, btScalar timeStep);
    static void myInternalTickCallback(btDynamicsWorld* dynamicsWorld, btScalar timeStep);

    void addTickContactInfo(const sp<CollisionObjectRef>& rigidBody, const sp<CollisionCallback>& callback, const sp<CollisionObjectRef>& contact, const V3& cp, const V3& normal);

private:
    sp<Stub> _stub;
    Map<sp<CollisionObjectRef>, ContactInfo> _contact_infos;
};

}
