#ifndef ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_
#define ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "core/base/object_pool.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/base/rigid_body.h"
#include "app/impl/collider/axis_segments.h"
#include "app/inf/collider.h"
#include "app/util/tinyc2_util.h"

namespace ark {

class ColliderImpl : public Collider {
public:
    ColliderImpl(const sp<Tracker>& tracker, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Tracker>> _tracker;

    };

public:
    class RigidBodyImpl;
    class RigidBodyShadow;

    struct ShapeManifest {
        V2 unit;
        std::vector<C2Shape> shapes;
    };

    struct Stub {
        Stub(const sp<Tracker>& tracker, const document& manifest, ResourceLoaderContext& resourceLoaderContext);

        void remove(const RigidBody& rigidBody);

        sp<RigidBodyImpl> createRigidBody(Collider::BodyType type, int32_t shape, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate, const sp<Disposed>& disposed, const sp<Stub>& self);
        const sp<RigidBodyShadow>& ensureRigidBody(int32_t id) const;
        const sp<RigidBodyShadow> findRigidBody(int32_t id) const;

        sp<Tracker> _tracker;

        std::unordered_map<int32_t, sp<RigidBodyShadow>> _rigid_bodies;
        std::unordered_map<int32_t, ShapeManifest> _c2_shapes;
        int32_t _rigid_body_base_id;
        ObjectPool _object_pool;

    private:
        void loadShapes(const document& manifest);

    };

    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t id, Collider::BodyType type, const sp<Vec>& position, const sp<Size>& size, const sp<Rotate>& rotate, const sp<Disposed>& disposed);

        virtual void dispose() override;

        void makeAABB();
        void makeBall();
        void makeBox();
        void setShapes(const std::vector<C2Shape>& shape, const V2& scale);

        bool isDisposed() const;

        void collision(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V& position, const Rect& aabb);

        void dispose(ColliderImpl::Stub& stub);

    private:
        Rect makeRigidBodyAABB() const;

    private:
        C2RigidBody _c2_rigid_body;

        std::unordered_set<int32_t> _contacts;
        bool _dispose_requested;

        friend class RigidBodyImpl;
    };

    class RigidBodyImpl : public RigidBody, Implements<RigidBodyImpl, RigidBody, Holder> {
    public:
        RigidBodyImpl(const sp<ColliderImpl::Stub>& collider, const sp<RigidBodyShadow>& shadow);
        ~RigidBodyImpl() override;

        virtual void dispose() override;

        const sp<RigidBodyShadow>& shadow() const;

    private:
        sp<ColliderImpl::Stub> _collider;
        sp<RigidBodyShadow> _shadow;
    };

private:
    sp<Stub> _stub;
    sp<ResourceLoaderContext> _resource_loader_context;
};

}

#endif
