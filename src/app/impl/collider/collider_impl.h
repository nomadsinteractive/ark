#pragma once

#include "core/inf/updatable.h"
#include "core/inf/variable.h"
#include "core/impl/builder/safe_builder.h"
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
    ColliderImpl(Vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase, RenderController& renderController);

    Rigidbody::Impl createBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded) override;
    sp<Shape> createShape(const NamedHash& type, sp<Vec3> size, sp<Vec3> origin) override;
    Vector<RayCastManifold> rayCast(V3 from, V3 to, const sp<CollisionFilter>& collisionFilter) override;

//  [[plugin::resource-loader]]
    class BUILDER final : public Builder<Collider> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<Collider> build(const Scope& args) override;

    private:
        Vector<std::pair<sp<Builder<BroadPhrase>>, SafeBuilder<CollisionFilter>>> _broad_phrases;
        sp<Builder<NarrowPhrase>> _narrow_phrase;
        sp<RenderController> _render_controller;
    };

    class RigidbodyImpl;

    struct Stub final : Updatable {
        Stub(Vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> broadPhrases, sp<NarrowPhrase> narrowPhrase);

        Vector<RayCastManifold> rayCast(V3 from, V3 to, const sp<CollisionFilter>& collisionFilter) const;

        void requestRigidBodyRemoval(int32_t rigidBodyId);

        sp<RigidbodyImpl> createRigidBody(Rigidbody::BodyType type, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded);

        sp<Ref> toRigidbodyRef(RefId refid) const;

        const sp<NarrowPhrase>& narrowPhrase() const;

        void updateBroadPhraseCandidate(RefId id, const V3& position, const V3& size) const;
        void removeBroadPhraseCandidate(RefId id);

        bool update(uint64_t timestamp) override;

    private:
        void broadPhraseSearch(BroadPhraseCallback& callback, V3 position, V3 size, const sp<CollisionFilter>& collisionFilter) const;

    private:
        Vector<std::pair<sp<BroadPhrase>, sp<CollisionFilter>>> _broad_phrases;
        sp<NarrowPhrase> _narrow_phrase;

        HashMap<RefId, sp<Ref>> _rigid_bodies;
        Set<const Ref*> _dirty_rigid_body_refs;

        Set<RefId> _phrase_discard;
        Set<RefId> _phrase_remove;

        friend class RigidbodyImpl;
    };

private:
    sp<Stub> _stub;
};

}
