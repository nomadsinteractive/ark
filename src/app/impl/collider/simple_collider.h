#ifndef ARK_APP_IMPL_COLLIDER_SIMPLE_COLLIDER_H_
#define ARK_APP_IMPL_COLLIDER_SIMPLE_COLLIDER_H_

#include <set>
#include <vector>
#include <unordered_map>

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "renderer/forwarding.h"

#include "app/inf/collider.h"
#include "app/base/rigid_body.h"
#include "app/impl/collider/static_segments.h"

namespace ark {

class SimpleCollider : public Collider {
public:
    SimpleCollider(const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RigidBody> createBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

    };

    class RigidBodyImpl;
    class RigidBodyShadow;

public:
    class Stub {
    public:
        Stub();

        void insert(const sp<RigidBodyImpl>& rigidObject);
        void remove(const RigidBodyImpl& rigidBody);

        sp<RigidBodyImpl> createRigidBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const sp<Stub>& self);
        const sp<RigidBodyShadow>& findRigidBody(uint32_t id) const;

        StaticSegments _x_axis_segment;
        StaticSegments _y_axis_segment;

        std::unordered_map<uint32_t, sp<RigidBodyShadow>> _rigid_bodies;
        uint32_t _rigid_body_base_id;
    };

    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t id, Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& pos, const sp<Size>& size);

        virtual void dispose() override;
        virtual const sp<CollisionCallback>& collisionCallback() const override;
        virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) override;

        void setPosition(const V& pos);

    private:
        sp<VV::Impl> _position;
        sp<CollisionCallback> _collision_callback;
    };

    class RigidBodyImpl : public RigidBody {
    public:
        RigidBodyImpl(const sp<VV>& position, const sp<Stub>& collider, const sp<RigidBodyShadow>& shadow);
        ~RigidBodyImpl();

        virtual void dispose() override;
        virtual const sp<CollisionCallback>& collisionCallback() const override;
        virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) override;

        void setPosition(const sp<VV>& position);
        void collision(const Rect& rect);
        void update();

    private:
        void beginContact(const sp<RigidBody>& rigidBody);
        void endContact(const sp<RigidBody>& rigidBody);

    private:
        sp<SimpleCollider::Stub> _collider;
        sp<RigidBodyShadow> _shadow;

        std::set<uint32_t> _contacts;
    };

private:
    sp<Stub> _stub;
    sp<ResourceLoaderContext> _resource_loader_context;
};

}

#endif
