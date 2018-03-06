#ifndef ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_
#define ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_

#include <set>
#include <unordered_map>

#include "core/base/object_pool.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "renderer/forwarding.h"

#include "app/base/rigid_body.h"
#include "app/impl/collider/axis_segments.h"
#include "app/inf/collider.h"

namespace ark {

class ColliderImpl : public Collider {
public:
    ColliderImpl(const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size) override;

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
    struct Stub {
        Stub();

        void insert(const sp<RigidBodyImpl>& rigidObject);
        void remove(const RigidBodyImpl& rigidBody);

        sp<RigidBodyImpl> createRigidBody(Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<Stub>& self);
        const sp<RigidBodyShadow>& ensureRigidBody(uint32_t id) const;
        const sp<RigidBodyShadow> findRigidBody(uint32_t id) const;

        AxisSegments _x_axis_segment;
        AxisSegments _y_axis_segment;

        std::unordered_map<uint32_t, sp<RigidBodyShadow>> _rigid_bodies;
        uint32_t _rigid_body_base_id;

        ObjectPool _object_pool;
    };

    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t id, Collider::BodyType type, const V& pos, const sp<Size>& size);

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
        sp<ColliderImpl::Stub> _collider;
        sp<RigidBodyShadow> _shadow;

        std::set<uint32_t> _contacts;
        bool _disposed;
    };

private:
    sp<Stub> _stub;
    sp<ResourceLoaderContext> _resource_loader_context;
};

}

#endif
