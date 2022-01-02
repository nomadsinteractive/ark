#ifndef ARK_APP_IMPL_NARROW_PHRASE_NARROW_PHRASE_CUTE_C2_H_
#define ARK_APP_IMPL_NARROW_PHRASE_NARROW_PHRASE_CUTE_C2_H_

#include <unordered_map>
#include <vector>

#include "core/inf/builder.h"

#include "renderer/forwarding.h"

#include "app/inf/narrow_phrase.h"
#include "app/util/shape_cute_c2.h"

namespace ark {

class NarrowPhraseCuteC2 : public NarrowPhrase {
public:
    NarrowPhraseCuteC2(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void addAABBShape(int32_t id, const Rect& aabb) override;
    virtual void addBallShape(int32_t id, const V2& position, float radius) override;
    virtual void addBoxShape(int32_t id, const Rect& bounds) override;
    virtual void addCapsuleShape(int32_t id, const V2& p1, const V2& p2, float radius) override;
    virtual void addPolygonShape(int32_t id, const std::vector<V2>& vertices) override;
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

private:
    void addPolygonShape(int32_t id, const c2Poly& poly);
    void toRay(const V2& from, const V2& to, c2Ray& ray) const;
    void loadShapes(const document& manifest);

private:
    std::unordered_map<int32_t, std::vector<ShapeCuteC2>> _shapes;
};

}

#endif
