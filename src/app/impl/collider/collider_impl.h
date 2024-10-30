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
#include "app/base/rigidbody.h"
#include "app/impl/collider/axis_segments.h"
#include "app/inf/collider.h"
#include "app/inf/broad_phrase.h"
#include "app/util/rigid_body_def.h"

namespace ark {

class ColliderImpl final : public Collider {
public:
    ColliderImpl(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrase, sp<NarrowPhrase> narrowPhrase, RenderController& renderController);

    sp<Rigidbody> createBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded) override;
    sp<Shape> createShape(const NamedType& type, sp<Vec3> size) override;
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

    struct Stub : public Updatable {
        Stub(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase);

        std::vector<RayCastManifold> rayCast(const V2& from, const V2& to, const sp<CollisionFilter>& collisionFilter) const;

        void requestRigidBodyRemoval(int32_t rigidBodyId);

        sp<RigidBodyImpl> createRigidBody(Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded);

        std::vector<sp<Ref>> toRigidBodyRefs(const std::unordered_set<BroadPhrase::IdType>& candidateSet, uint32_t filter) const;
        std::vector<BroadPhrase::Candidate> toBroadPhraseCandidates(const std::unordered_set<BroadPhrase::IdType>& candidateSet, uint32_t filter) const;

        void resolveCandidates(const Rigidbody& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, const Rigidbody& callback, std::set<BroadPhrase::IdType>& c);

        const sp<NarrowPhrase>& narrowPhrase() const;

        void updateBroadPhraseCandidate(BroadPhrase::IdType id, const V3& position, const V3& aabb) const;
        void removeBroadPhraseCandidate(BroadPhrase::IdType id);

        bool update(uint64_t timestamp) override;

    private:
        BroadPhrase::Result broadPhraseSearch(const V3& position, const V3& aabb, const sp<CollisionFilter>& collisionFilter) const;
        BroadPhrase::Result broadPhraseRayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) const;

    private:
        std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> _broad_phrases;
        sp<NarrowPhrase> _narrow_phrase;

        std::unordered_map<uintptr_t, sp<Ref>> _rigid_bodies;
        std::vector<sp<Ref>> _rigid_body_refs;

        std::set<BroadPhrase::IdType> _phrase_dispose;
        std::set<BroadPhrase::IdType> _phrase_remove;

        friend class RigidBodyImpl;
    };

    class RigidBodyImpl : public Rigidbody, Implements<RigidBodyImpl, Rigidbody, Wirable> {
    public:
        RigidBodyImpl(const ColliderImpl::Stub& stub, Collider::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded);

        bool update(uint64_t timestamp);

        void collisionTest(ColliderImpl::Stub& collider, const V3& position, const V3& size, const std::set<BroadPhrase::IdType>& removingIds);

        void doDispose(ColliderImpl::Stub& stub);

        const RigidbodyDef& bodyDef() const;
        const RigidbodyDef& updateBodyDef(NarrowPhrase& narrowPhrase, const SafeVar<Vec3>& size);

        BroadPhrase::Candidate toBroadPhraseCandidate() const;

    private:
        const ColliderImpl::Stub& _collider_stub;
        std::set<BroadPhrase::IdType> _dynamic_contacts;
        std::set<BroadPhrase::IdType> _static_contacts;

        RigidbodyDef _body_def;

        bool _position_updated;
        bool _size_updated;
    };

private:
    sp<Stub> _stub;
};

}
