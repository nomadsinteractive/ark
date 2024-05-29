#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/inf/variable.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/base/rigid_body.h"
#include "app/impl/collider/axis_segments.h"
#include "app/inf/collider.h"
#include "app/inf/broad_phrase.h"
#include "app/util/rigid_body_def.h"

namespace ark {

class ColliderImpl : public Collider {
public:
    ColliderImpl(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrase, sp<NarrowPhrase> narrowPhrase, RenderController& renderController);

    sp<RigidBody> createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotate, sp<Boolean> discarded) override;
    std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        std::vector<std::pair<sp<Builder<BroadPhrase>>, SafePtr<Builder<CollisionFilter>>>> _broad_phrases;
        sp<Builder<NarrowPhrase>> _narrow_phrase;
        sp<RenderController> _render_controller;
    };

public:
    class RigidBodyImpl;
    class RigidBodyShadow;

    struct Stub : public Updatable {
        Stub(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase);

        std::vector<RayCastManifold> rayCast(const V2& from, const V2& to, const sp<CollisionFilter>& collisionFilter) const;

        void requestRigidBodyRemoval(int32_t rigidBodyId);

        int32_t generateRigidBodyId();

        sp<RigidBodyImpl> createRigidBody(int32_t rigidBodyId, Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotate, sp<Boolean> discarded);

        const sp<RigidBodyShadow>& ensureRigidBody(int32_t id) const;
        sp<RigidBodyShadow> ensureRigidBody(int32_t id, uint32_t metaId, int32_t shapeId, const V3& position, bool isDynamicRigidBody) const;
        sp<RigidBodyShadow> findRigidBody(int32_t id) const;

        std::vector<sp<RigidBodyShadow>> toRigidBodyShadows(const std::unordered_set<BroadPhrase::IdType>& candidateSet, uint32_t filter) const;
        std::vector<BroadPhrase::Candidate> toBroadPhraseCandidates(const std::unordered_set<BroadPhrase::IdType>& candidateSet, uint32_t filter) const;

        void resolveCandidates(const sp<RigidBody>& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, bool isDynamicCandidates, RigidBody::Callback& callback, std::set<int32_t>& c);

        const sp<NarrowPhrase>& narrowPhrase() const;

        void updateBroadPhraseCandidate(int32_t id, const V3& position, const V3& aabb) const;
        void removeBroadPhraseCandidate(int32_t id);

        bool update(uint64_t timestamp) override;

    private:
        BroadPhrase::Result broadPhraseSearch(const V3& position, const V3& aabb, const sp<CollisionFilter>& collisionFilter) const;
        BroadPhrase::Result broadPhraseRayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) const;

    private:
        int32_t _rigid_body_base_id;
        std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> _broad_phrases;
        sp<NarrowPhrase> _narrow_phrase;

        std::unordered_map<uintptr_t, sp<RigidBodyShadow>> _rigid_bodies;

        std::vector<sp<RigidBodyShadow>> _rigid_body_refs;

        std::set<int32_t> _phrase_dispose;
        std::set<int32_t> _phrase_remove;

        friend class RigidBodyShadow;
    };

    class RigidBodyShadow : public RigidBody, public Updatable {
    public:
        RigidBodyShadow(const ColliderImpl::Stub& stub, uint32_t id, Collider::BodyType type, uint32_t metaId, sp<Shape> shape, sp<Vec3> position, sp<Rotation> rotation, SafeVar<Boolean> discarded);

        void dispose() override;
        bool update(uint64_t timestamp) override;

        void collisionTest(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V3& position, const V3& size, const std::set<int32_t>& removingIds);

        void doDispose(ColliderImpl::Stub& stub);

        const RigidBodyDef& bodyDef() const;
        const RigidBodyDef& updateBodyDef(NarrowPhrase& narrowPhrase, const SafeVar<Vec3>& size);

        BroadPhrase::Candidate toBroadPhraseCandidate() const;

    private:
        const ColliderImpl::Stub& _collider_stub;
        std::set<int32_t> _dynamic_contacts;
        std::set<int32_t> _static_contacts;

        RigidBodyDef _body_def;

        bool _position_updated;
        bool _size_updated;

        friend class RigidBodyImpl;
    };

    class RigidBodyImpl : public RigidBody, Implements<RigidBodyImpl, RigidBody, Wirable> {
    public:
        RigidBodyImpl(sp<RigidBodyShadow> shadow);

        void dispose() override;

    private:
        void doDispose();

    private:
        sp<ColliderImpl::Stub> _collider;
        sp<RigidBodyShadow> _shadow;
    };

private:
    sp<Stub> _stub;
};

}
