#ifndef ARK_PLUGIN_BOX2D_IMPL_WORLD_H_
#define ARK_PLUGIN_BOX2D_IMPL_WORLD_H_

#include <unordered_map>
#include <vector>

#include <Box2D/Box2D.h>

#include "core/base/bean_factory.h"
#include "core/base/object.h"
#include "core/base/object_pool.h"
#include "core/inf/builder.h"
#include "core/inf/runnable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/body_create_info.h"
#include "box2d/impl/joint.h"

namespace ark {
namespace plugin {
namespace box2d {

class ARK_PLUGIN_BOX2D_API World : public Object, public Runnable, public Collider, Implements<World, Object, Runnable, Collider> {
public:

    class Importer {
    public:
        virtual ~Importer() = default;

        virtual void import(World& world) = 0;

    };

public:
    World(const b2Vec2& gravity, float ppmX, float ppmY);
    DEFAULT_COPY_AND_ASSIGN(World);

    virtual void run() override;

//  [[script::bindings::auto]]
    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size = nullptr, const sp<Rotate>& rotate = nullptr) override;

    b2World& world() const;

    b2Body* createBody(const b2BodyDef& bodyDef) const;
    b2Body* createBody(Collider::BodyType type, const V& position, const sp<Size>& size, const BodyCreateInfo& createInfo) const;

//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]

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
    class BUILDER_IMPL1 : public Builder<World> {
    public:
        BUILDER_IMPL1(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<World> build(const sp<Scope>& args) override;

    private:
        void createBody(Collider::BodyType type, const sp<World>& world, const document& manifest, const sp<Scope>& args);

    private:
        BeanFactory _factory;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

        std::vector<sp<Builder<Importer>>> _importers;

        sp<Builder<Numeric>> _ppmx, _ppmy;
        sp<Builder<Numeric>> _gravity_x, _gravity_y;
        SafePtr<Builder<Boolean>> _expired;
    };

//  [[plugin::resource-loader("b2World")]]
    class BUILDER_IMPL2 : public Builder<Collider> {
    public:
        BUILDER_IMPL2(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        BUILDER_IMPL1 _delegate;
    };

    void setBodyManifest(int32_t id, const BodyCreateInfo& bodyManifest);

private:
    class ContactListenerImpl : public b2ContactListener {
    public:
        virtual void BeginContact(b2Contact* contact);
        virtual void EndContact(b2Contact* contact);

    private:
        ObjectPool _object_pool;
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
        Stub(const b2Vec2& gravity, float ppmX, float ppmY);

        virtual void run() override;

        float _ppm_x;
        float _ppm_y;

        float _time_step;
        int32_t _velocity_iterations;
        int32_t _position_iterations;
        int32_t _rigid_body_id_base;

        b2World _world;
        std::unordered_map<int32_t, BodyCreateInfo> _body_manifests;

        ContactListenerImpl _contact_listener;
        DestructionListenerImpl _destruction_listener;
    };

    int32_t genRigidBodyId() const;

private:
    sp<Stub> _stub;

    friend class BUILDER_IMPL1;
    friend class Body;
    friend class Importer;
};

}
}
}

#endif
