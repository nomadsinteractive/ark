#pragma once

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/inf/variable.h"
#include "core/types/safe_builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/components/rigidbody.h"
#include "app/impl/collider/axis_segments.h"
#include "app/inf/collider.h"
#include "app/inf/broad_phrase.h"

namespace ark {

class ColliderImpl final : public Collider {
public:
    ColliderImpl(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrase, sp<NarrowPhrase> narrowPhrase, RenderController& renderController);

    Rigidbody::Impl createBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded) override;
    sp<Shape> createShape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin) override;
    std::vector<RayCastManifold> rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) override;

    //  [[plugin::resource-loader]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        std::vector<std::pair<sp<Builder<BroadPhrase>>, SafeBuilder<CollisionFilter>>> _broad_phrases;
        sp<Builder<NarrowPhrase>> _narrow_phrase;
        sp<RenderController> _render_controller;
    };

public:
    class RigidbodyImpl;

    struct Stub : public Updatable {
        Stub(std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase);

        std::vector<RayCastManifold> rayCast(const V2& from, const V2& to, const sp<CollisionFilter>& collisionFilter) const;

        void requestRigidBodyRemoval(int32_t rigidBodyId);

        sp<RigidbodyImpl> createRigidBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded);

        std::vector<sp<Ref>> toRigidBodyRefs(const std::unordered_set<BroadPhrase::CandidateIdType>& candidateSet, uint32_t filter) const;
        std::vector<BroadPhrase::Candidate> toBroadPhraseCandidates(const std::unordered_set<BroadPhrase::CandidateIdType>& candidateSet) const;

        void resolveCandidates(const Rigidbody& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, std::set<BroadPhrase::CandidateIdType>& c);

        const sp<NarrowPhrase>& narrowPhrase() const;

        void updateBroadPhraseCandidate(BroadPhrase::CandidateIdType id, const V3& position, const V3& aabb) const;
        void removeBroadPhraseCandidate(BroadPhrase::CandidateIdType id);

        bool update(uint64_t timestamp) override;

    private:
        BroadPhrase::Result broadPhraseSearch(const V3& position, const V3& aabb, const sp<CollisionFilter>& collisionFilter) const;
        BroadPhrase::Result broadPhraseRayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter) const;

    private:
        std::vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> _broad_phrases;
        sp<NarrowPhrase> _narrow_phrase;

        std::unordered_map<BroadPhrase::CandidateIdType, sp<Ref>> _rigid_bodies;
        Set<const Ref*> _dirty_rigid_body_refs;

        std::set<BroadPhrase::CandidateIdType> _phrase_dispose;
        std::set<BroadPhrase::CandidateIdType> _phrase_remove;

        friend class RigidbodyImpl;
    };

private:
    sp<Stub> _stub;
};

}
