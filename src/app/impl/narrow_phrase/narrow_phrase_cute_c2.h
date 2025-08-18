#pragma once

#include "core/inf/builder.h"

#include "renderer/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/narrow_phrase.h"

#include "app/util/shape_cute_c2.h"

namespace ark {

class NarrowPhraseCuteC2 final : public NarrowPhrase {
public:
    NarrowPhraseCuteC2(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    ShapeDef createShapeDef(HashId shapeId, Optional<V3> size) override;
    RigidbodyDef makeBodyDef(HashId shapeId, sp<Vec3> size) override;

    Ray toRay(const V2& from, const V2& to) override;

    bool collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold) override;
    Optional<RayCastManifold> rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate) override;

//  [[plugin::resource-loader("cute-c2")]]
    class BUILDER final : public Builder<NarrowPhrase> {
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
        BodyDefCuteC2(const V2& size, const V2& pivot, Vector<ShapeCuteC2> shapes);
        BodyDefCuteC2(const document& manifest, float ppu);
        DEFAULT_COPY_AND_ASSIGN(BodyDefCuteC2);

        const V2& size() const;
        const V2& pivot() const;

        const Vector<ShapeCuteC2>& shapes() const;

        void resize(const V2& size);

    private:
        V2 _size;
        V2 _pivot;
        Vector<ShapeCuteC2> _shapes;
    };

    static sp<BodyDefCuteC2> makeBodyAABB(const Rect& aabb);
    static sp<BodyDefCuteC2> makeBodyBall(const V2& position, float radius);
    static sp<BodyDefCuteC2> makeBodyBox(const Rect& bounds);
    static sp<BodyDefCuteC2> makeBodyCapsule(const V2& p1, const V2& p2, float radius);

    void loadShapes(const document& manifest, float ppu);

    sp<BodyDefCuteC2> findBodyDef(HashId shapeId) const;
    sp<BodyDefCuteC2> ensureBodyDef(const BroadPhrase::Candidate& candidate) const;
    sp<BodyDefCuteC2> createShapeImplementation(HashId shapeId, const sp<Vec3>& size) const;

private:
    HashMap<HashId, sp<BodyDefCuteC2>> _predefined_shapes;
};

}
