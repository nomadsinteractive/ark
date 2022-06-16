#ifndef ARK_APP_IMPL_NARROW_PHRASE_NARROW_PHRASE_CUTE_C2_H_
#define ARK_APP_IMPL_NARROW_PHRASE_NARROW_PHRASE_CUTE_C2_H_

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

    virtual Box makeAABBShape(const Rect& aabb) override;
    virtual Box makeBallShape(const V2& position, float radius) override;
    virtual Box makeBoxShape(const Rect& bounds) override;
    virtual Box makeCapsuleShape(const V2& p1, const V2& p2, float radius) override;
    virtual Box makePolygonShape(const std::vector<V2>& vertices) override;

    virtual Ray toRay(const V2& from, const V2& to) override;

    virtual bool collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold) override;
    virtual bool rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate, RayCastManifold& rayCastManifold) override;

public:
//  [[plugin::resource-loader("cute-c2")]]
    class BUILDER : public Builder<NarrowPhrase> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<NarrowPhrase> build(const Scope& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
    };

    static ShapeCuteC2 makeAABBShapeImpl(const Rect& bounds);
    static ShapeCuteC2 makeBallShapeImpl(const V2& position, float radius);
    static ShapeCuteC2 makeBoxShapeImpl(const Rect& bounds);
    static ShapeCuteC2 makeCapsuleShapeImpl(const V2& p1, const V2& p2, float radius);
    static ShapeCuteC2 makePolygonShapeImpl(const std::vector<V2>& vertices);

private:
    void toRay(const V2& from, const V2& to, c2Ray& ray) const;
    void loadShapes(const document& manifest);

    const CollisionFilter& getCollisionFilter(const CollisionFilter& oneFilter, const sp<CollisionFilter>& specifiedFilter);

    const std::vector<sp<ShapeCuteC2>>& ensureShape(const BroadPhrase::Candidate& candidate, std::vector<sp<ShapeCuteC2>>& predefined) const;

    std::vector<sp<ShapeCuteC2>> toCuteC2Shapes(const std::vector<Box>& boxes) const;

private:
    std::unordered_map<int32_t, sp<BodyDefCuteC2>> _body_defs;
};

}

#endif
