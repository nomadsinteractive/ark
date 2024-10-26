#pragma once

#include <unordered_map>
#include <vector>

#include <Box2D/Box2D.h>

#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/implements.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"
#include "core/inf/storage.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/body_create_info.h"
#include "box2d/impl/joint.h"

namespace ark::plugin::box2d {

//[[script::bindings::name("World")]]
class ARK_PLUGIN_BOX2D_API ColliderBox2D final : public Runnable, public Collider, Implements<ColliderBox2D, Runnable, Collider> {
public:
    typedef Importer<ColliderBox2D> RigidBodyImporter;

public:
    ColliderBox2D(const b2Vec2& gravity, const V2& pixelPerMeter);
    DEFAULT_COPY_AND_ASSIGN(ColliderBox2D);

    void run() override;

//  [[script::bindings::auto]]
    sp<Rigidbody> createBody(Collider::BodyType type, sp<ark::Shape> shape, sp<Vec3> position, sp<Rotation> rotate = nullptr, sp<Boolean> discarded = nullptr) override;
//  [[script::bindings::auto]]
    std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

    b2World& world() const;

    b2Body* createBody(const b2BodyDef& bodyDef) const;
    b2Body* createBody(Collider::BodyType type, const V3& position, const V3& size, const BodyCreateInfo& createInfo) const;

//  [[script::bindings::auto]]
    float toPixelX(float meterX) const;
//  [[script::bindings::auto]]
    float toPixelY(float meterY) const;

//  [[script::bindings::auto]]
    float toMeterX(float pixelX) const;
//  [[script::bindings::auto]]
    float toMeterY(float pixelY) const;

//  [[script::bindings::property]]
    int32_t bodyCount() const;

    void track(const sp<Joint::Stub>& joint) const;

//  [[plugin::resource-loader]]
    class BUILDER_IMPL1 : public Builder<ColliderBox2D> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ColliderBox2D> build(const Scope& args) override;

    private:
        void createBody(Collider::BodyType type, const sp<ColliderBox2D>& world, const document& manifest, const Scope& args);

    private:
        BeanFactory _factory;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        std::vector<std::pair<sp<Builder<RigidBodyImporter>>, String>> _importers;

        sp<Builder<Vec2>> _ppm;
        sp<Builder<Vec2>> _gravity;
        SafePtr<Builder<Boolean>> _disposed;
    };

//  [[plugin::resource-loader("b2World")]]
    class BUILDER_IMPL2 : public Builder<Collider> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        BUILDER_IMPL1 _delegate;
    };

    void setBodyManifest(int32_t id, const BodyCreateInfo& bodyManifest);

private:
    class ContactListenerImpl : public b2ContactListener {
    public:
        virtual void BeginContact(b2Contact* contact);
        virtual void EndContact(b2Contact* contact);
    };

    class DestructionListenerImpl : public b2DestructionListener {
    public:
        virtual void SayGoodbye(b2Joint* joint) override;
        virtual void SayGoodbye(b2Fixture* fixture) override;

        void track(const sp<Joint::Stub>& joint);

    private:
        std::unordered_map<b2Joint*, WeakPtr<Joint::Stub>> _joints;
    };

    struct Stub : public Runnable {
        Stub(const b2Vec2& gravity, const V2& pixelPerMeter);

        virtual void run() override;

        float _ppm_x;
        float _ppm_y;

        float _time_step;
        int32_t _velocity_iterations;
        int32_t _position_iterations;

        b2World _world;
        std::unordered_map<TypeId, BodyCreateInfo> _body_manifests;

        ContactListenerImpl _contact_listener;
        DestructionListenerImpl _destruction_listener;
    };

private:
    sp<Stub> _stub;

    friend class BUILDER_IMPL1;
    friend class RigidbodyBox2D;
};

}
