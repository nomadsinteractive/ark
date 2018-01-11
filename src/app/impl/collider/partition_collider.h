#ifndef ARK_APP_IMPL_COLLIDER_PARTITION_COLLIDER_H_
#define ARK_APP_IMPL_COLLIDER_PARTITION_COLLIDER_H_

#include <set>
#include <vector>
#include <unordered_map>

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/weak_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"
#include "app/base/rigid_body.h"

namespace ark {

class PartitionCollider : public Collider {
public:
    PartitionCollider(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Partition>& partition);

    virtual sp<RigidBody> createBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size) override;

//  [[plugin::resource-loader("partition")]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Partition>> _partition;

    };

    class RigidBodyImpl;

public:
    class Stub {
    public:
        Stub(const sp<Partition>& partition);

        void remove(const RigidBodyImpl& rigidBody);

        sp<RigidBodyImpl> createRigidBody(Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const sp<Stub>& self);
        const sp<RigidBodyImpl>& findRigidBody(uint32_t id) const;

        std::unordered_map<uint32_t, sp<RigidBodyImpl>> _rigid_bodies;
        sp<Partition> _partition;
        float _search_radius;
    };

    class RigidBodyImpl : public RigidBody {
    public:
        RigidBodyImpl(uint32_t id, Collider::BodyType type, Collider::BodyShape shape, const sp<VV>& position, const sp<Size>& size, const WeakPtr<PartitionCollider::Stub>& stub);
        ~RigidBodyImpl();

        virtual void dispose() override;
        virtual const sp<CollisionCallback>& collisionCallback() const override;
        virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) override;

        void setPosition(const sp<VV>& position);

        V xy() const;

        void collision(const Rect& rect);

    public:
        struct Stub {
            Stub(const WeakPtr<PartitionCollider::Stub>& colliderStub, const sp<VV>& position);

            void collision(uint32_t selfId, const Rect& rect, float searchRadius);

            WeakPtr<PartitionCollider::Stub> _collider_stub;
            sp<VV> _position;
            sp<CollisionCallback> _collision_callback;

        private:
            void beginContact(const sp<RigidBodyImpl>& rigidBody);
            void endContact(const sp<RigidBodyImpl>& rigidBody);

        private:
            std::set<uint32_t> _contacts;
        };

        const sp<RigidBodyImpl::Stub>& stub() const;

    private:
        sp<Stub> _stub;
        bool _disposed;
    };

private:
    sp<Stub> _stub;
    sp<ResourceLoaderContext> _resource_loader_context;
};

}

#endif
