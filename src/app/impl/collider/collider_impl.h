#ifndef ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_
#define ARK_APP_IMPL_COLLIDER_COLLIDER_IMPL_H_

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include <core/inf/runnable.h>

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
#include "app/util/rigid_body_def.h"

namespace ark {

class ColliderImpl : public Collider {
public:
    ColliderImpl(std::vector<sp<BroadPhrase>> broadPhrase, sp<NarrowPhrase> narrowPhrase, const document& manifest, const sp<RenderController>& renderController);

    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<Vec3>& position, const sp<Size>& size, const sp<Rotation>& rotate, sp<Boolean> disposed) override;
    virtual std::vector<RayCastManifold> rayCast(const V3& from, const V3& to) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Collider> build(const Scope& args) override;

    private:
        document _manifest;
        std::vector<sp<Builder<BroadPhrase>>> _broad_phrases;
        sp<Builder<NarrowPhrase>> _narrow_phrase;
        sp<RenderController> _render_controller;
    };

public:
    class RigidBodyImpl;
    class RigidBodyShadow;

    struct Stub : public Runnable {
        Stub(std::vector<sp<BroadPhrase>> broadPhrases, sp<NarrowPhrase> narrowPhrase);

        std::vector<RayCastManifold> rayCast(const V2& from, const V2& to) const;

        void removeRigidBody(int32_t rigidBodyId);

        int32_t generateRigidBodyId();

        sp<RigidBodyImpl> createRigidBody(int32_t rigidBodyId, Collider::BodyType type, int32_t shape, sp<Vec3> position, sp<Size> size, sp<Rotation> rotate, sp<Disposed> disposed, sp<ColliderImpl::Stub> self);

        const sp<RigidBodyShadow>& ensureRigidBody(int32_t id) const;
        sp<RigidBodyShadow> ensureRigidBody(int32_t id, int32_t shapeId, const V3& position, bool isDynamicRigidBody) const;
        sp<RigidBodyShadow> findRigidBody(int32_t id) const;

        std::vector<sp<RigidBodyShadow>> toRigidBodyShadows(const std::unordered_set<int32_t>& candidateSet, uint32_t filter) const;
        std::vector<BroadPhrase::Candidate> toBroadPhraseCandidates(const std::unordered_set<int32_t>& candidateSet, uint32_t filter) const;
        BroadPhrase::Candidate toBroadPhraseCandidate(const RigidBodyShadow& rigidBody) const;

        void resolveCandidates(const sp<RigidBody>& self, const BroadPhrase::Candidate& candidateSelf, const std::vector<BroadPhrase::Candidate>& candidates, bool isDynamicCandidates, RigidBody::Callback& callback, std::set<int32_t>& c);

        const sp<NarrowPhrase>& narrowPhrase() const;

        void updateBroadPhraseCandidate(int32_t id, const V3& position, const V3& aabb);
        void removeBroadPhraseCandidate(int32_t id);

        virtual void run() override;

    private:
        BroadPhrase::Result broadPhraseSearch(const V3& position, const V3& aabb) const;
        BroadPhrase::Result broadPhraseRayCast(const V3& from, const V3& to) const;

    private:
        int32_t _rigid_body_base_id;
        std::vector<sp<BroadPhrase>> _broad_phrases;
        sp<NarrowPhrase> _narrow_phrase;
        std::unordered_map<int32_t, sp<RigidBodyShadow>> _rigid_bodies;

        std::vector<int32_t> _phrase_dispose;
        std::vector<int32_t> _phrase_remove;

        friend class RigidBodyShadow;
    };

    class RigidBodyShadow : public RigidBody {
    public:
        RigidBodyShadow(uint32_t id, Collider::BodyType type, int32_t shape, sp<Vec3> position, sp<Size> size, sp<Rotation> rotation, sp<Disposed> disposed);

        virtual void dispose() override;

        bool isDisposed() const;

        void collision(const sp<RigidBodyShadow>& self, ColliderImpl::Stub& collider, const V3& position, const V3& size);

        void doDispose(ColliderImpl::Stub& stub);

        const RigidBodyDef& bodyDef() const;
        const RigidBodyDef& updateBodyDef(NarrowPhrase& narrowPhrase, const sp<Size>& size);

    private:
        std::set<int32_t> _dynamic_contacts;
        std::set<int32_t> _static_contacts;
        bool _dispose_requested;

        RigidBodyDef _body_def;

        friend class RigidBodyImpl;
    };

    class RigidBodyImpl : public RigidBody, Implements<RigidBodyImpl, RigidBody, Holder> {
    public:
        RigidBodyImpl(sp<ColliderImpl::Stub> collider, sp<RigidBodyShadow> shadow);
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
    sp<RenderController> _render_controller;
};

}

#endif
