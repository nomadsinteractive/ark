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
#include "app/util/tinyc2_util.h"

namespace ark {

class ColliderImpl : public Collider {
public:
    ColliderImpl(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size, const sp<Transform>& transform) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;

    };

    class RigidBodyImpl;
    class RigidBodyShadow;

public:
    class Axises {
    public:

        void insert(const RigidBody& rigidBody);
        void remove(const RigidBody& rigidBody);
        void update(uint32_t id, const V2& position, const Rect& aabb);

        std::set<uint32_t> findCandidates(const Rect& aabb) const;

    private:
        AxisSegments _x_axis_segment;
        AxisSegments _y_axis_segment;
    };

    struct Stub {
        Stub(const document& manifest);

        void remove(const RigidBodyImpl& rigidBody);

        sp<RigidBodyImpl> createRigidBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size, const sp<Transform>& transform, const sp<Stub>& self);
        const sp<RigidBodyShadow>& ensureRigidBody(uint32_t id) const;
        const sp<RigidBodyShadow> findRigidBody(uint32_t id) const;

        std::unordered_map<uint32_t, sp<RigidBodyShadow>> _rigid_bodies;
        std::unordered_map<int32_t, std::pair<C2_TYPE, C2Shape>> _c2_shapes;
        uint32_t _rigid_body_base_id;
        sp<Axises> _axises;

        ObjectPool _object_pool;

    private:
        void loadShapes(const document& manifest);

    };

    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t id, Collider::BodyType type, const V& pos, const sp<Size>& size, const sp<Transform>& transform);

        virtual void dispose() override;
        virtual const sp<CollisionCallback>& collisionCallback() const override;
        virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) override;

        void makeAABB();
        void makeBall();
        void makeBox();
        void makeShape(C2_TYPE type, const C2Shape& shape);

        void setPosition(const V& pos);
        bool disposed() const;

        void collision(ColliderImpl::Stub& collider, const Rect& aabb);

    private:
        void beginContact(const sp<RigidBody>& rigidBody);
        void endContact(const sp<RigidBody>& rigidBody);

        Rect makeRigidBodyAABB() const;

    private:
        sp<VV::Impl> _position;
        C2RigidBody _c2_rigid_body;
        sp<CollisionCallback> _collision_callback;

        std::set<uint32_t> _contacts;
        bool _disposed;

        friend class RigidBodyImpl;
    };

    class RigidBodyImpl : public RigidBody {
    public:
        RigidBodyImpl(const sp<VV>& position, const sp<Stub>& collider, const sp<RigidBodyShadow>& shadow);
        ~RigidBodyImpl();

        virtual void dispose() override;
        virtual const sp<CollisionCallback>& collisionCallback() const override;
        virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) override;

        const sp<RigidBodyShadow>& shadow() const;

        void setPosition(const sp<VV>& position);
        void collision(const Rect& rect);

    private:
        sp<ColliderImpl::Stub> _collider;
        sp<RigidBodyShadow> _shadow;

        std::set<uint32_t> _contacts;
    };

private:
    sp<Stub> _stub;
    sp<ResourceLoaderContext> _resource_loader_context;
};

}

#endif
