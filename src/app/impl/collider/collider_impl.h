#ifndef ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_
#define ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/base/rigid_body.h"
#include "app/impl/collider/axis_segments.h"
#include "app/inf/collider.h"
#include "app/inf/broad_phrase.h"

namespace ark {

class ColliderImpl : public Collider {
public:
    ColliderImpl(std::vector<sp<BroadPhrase>> broadPhrase, sp<NarrowPhrase> narrowPhrase, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate) override;
    virtual std::vector<RayCastManifold> rayCast(const V3& from, const V3& to) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
        std::vector<sp<Builder<BroadPhrase>>> _broad_phrases;
        sp<Builder<NarrowPhrase>> _narrow_phrase;
    };

public:
    class RigidBodyImpl;
    class RigidBodyShadow;

    struct Stub {
        Stub(std::vector<sp<BroadPhrase>> broadPhrases, sp<NarrowPhrase> narrowPhrase, const document& manifest, ResourceLoaderContext& resourceLoaderContext);

        BroadPhrase::Result search(const V3& position, const V3& aabb) const;
        std::vector<RayCastManifold> rayCast(const V2& from, const V2& to) const;

        void remove(const RigidBody& rigidBody);

        int32_t generateRigidBodyId();

        sp<RigidBodyImpl> createRigidBody(int32_t rigidBodyId, Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate, const sp<Disposed>& disposed, const sp<Stub>& self);
        const sp<RigidBodyShadow>& ensureRigidBody(int32_t id) const;
        sp<RigidBodyShadow> ensureRigidBody(int32_t id, int32_t shapeId, const V3& position, bool isDynamicRigidBody) const;

        sp<RigidBodyShadow> findRigidBody(int32_t id) const;

        std::vector<BroadPhrase::Candidate> toDynamicCandidates(const std::unordered_set<int32_t>& candidateSet) const;

        void resolveCandidates(const sp<RigidBody>& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, bool isDynamicCandidates, RigidBody::Callback& callback, std::set<int32_t>& c);

        const sp<NarrowPhrase>& narrowPhrase() const;

    private:
        void addAABBShape(uint32_t id, const RigidBodyShadow& rigidBody);
        void addBoxShape(uint32_t id, const RigidBodyShadow& rigidBody);
        void addBallShape(uint32_t id, const RigidBodyShadow& rigidBody);
        void addCapsuleShape(uint32_t id, const RigidBodyShadow& rigidBody);
        void addPolyShape(uint32_t id, const RigidBodyShadow& rigidBody);

        sp<Vec3> createBroadPhrasePosition(int32_t id, const sp<Vec3>& position, const sp<Vec3>& aabb);

    private:
        int32_t _rigid_body_base_id;
        std::vector<sp<BroadPhrase>> _broad_phrases;
        sp<NarrowPhrase> _narrow_phrase;
        std::unordered_map<int32_t, sp<RigidBodyShadow>> _rigid_bodies;
    };

    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t id, Collider::BodyType type, int32_t shape, sp<Vec3> position, sp<Size> size, sp<Rotation> rotate, sp<Disposed> disposed);

        virtual void dispose() override;

        bool isDisposed() const;

        void collision(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V3& position, const V3& size);

        void doDispose(ColliderImpl::Stub& stub);

        const std::vector<Box>& shapes() const;

        void updateShapes(NarrowPhrase& narrowPhrase, const V3& size);

    private:
        std::set<int32_t> _dynamic_contacts;
        std::set<int32_t> _static_contacts;
        bool _dispose_requested;

        std::vector<Box> _shapes;

        friend class RigidBodyImpl;
    };

    class RigidBodyImpl : public RigidBody, Implements<RigidBodyImpl, RigidBody, Holder> {
    public:
        RigidBodyImpl(const sp<ColliderImpl::Stub>& collider, const sp<RigidBodyShadow>& shadow);
        ~RigidBodyImpl() override;

        virtual void dispose() override;

        const sp<RigidBodyShadow>& shadow() const;

    private:
        void doDispose();

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
