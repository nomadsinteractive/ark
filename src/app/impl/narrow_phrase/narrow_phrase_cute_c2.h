#pragma once

#include <unordered_map>
#include <vector>

#include "core/inf/builder.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/narrow_phrase.h"

#include "app/util/shape_cute_c2.h"

namespace ark {

class NarrowPhraseCuteC2 : public NarrowPhrase {
public:
    NarrowPhraseCuteC2(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    RigidbodyDef makeBodyDef(TypeId shapeId, const SafeVar<Vec3>& size) override;

    Ray toRay(const V2& from, const V2& to) override;

    bool collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold) override;
    Optional<RayCastManifold> rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate) override;

public:
//  [[plugin::resource-loader("cute-c2")]]
    class BUILDER : public Builder<NarrowPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        sp<NarrowPhrase> build(const Scope& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:

    class BodyDefCuteC2 {
    public:
        BodyDefCuteC2(const V2& size, const V2& pivot, std::vector<ShapeCuteC2> shapes);
        BodyDefCuteC2(const document& manifest, float ppu);
        DEFAULT_COPY_AND_ASSIGN(BodyDefCuteC2);

        const V2& size() const;
        const V2& pivot() const;

        const std::vector<ShapeCuteC2>& shapes() const;

        void resize(const V2& size);

    private:
        V2 _size;
        V2 _pivot;
        std::vector<ShapeCuteC2> _shapes;

    };

    static sp<BodyDefCuteC2> makeBodyAABB(const Rect& aabb);
    static sp<BodyDefCuteC2> makeBodyBall(const V2& position, float radius);
    static sp<BodyDefCuteC2> makeBodyBox(const Rect& bounds);
    static sp<BodyDefCuteC2> makeBodyCapsule(const V2& p1, const V2& p2, float radius);

    void loadShapes(const document& manifest, float ppu);

    sp<BodyDefCuteC2> findBodyDef(HashId shapeId) const;
    sp<BodyDefCuteC2> ensureBodyDef(const BroadPhrase::Candidate& candidate) const;

private:
    HashMap<HashId, sp<BodyDefCuteC2>> _body_defs;
};

}
