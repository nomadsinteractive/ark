#ifndef ARK_APP_INF_NARROW_PHRASE_H_
#define ARK_APP_INF_NARROW_PHRASE_H_

#include <unordered_set>

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/broad_phrase.h"

namespace ark {

class ARK_API NarrowPhrase {
public:
    class Ray {
    public:
        template<class T> const T* data() const {
            return reinterpret_cast<const T*>(_data);
        }

        template<class T> T* data() {
            return reinterpret_cast<T*>(_data);
        }

    private:
        int32_t _data[8];
    };
public:
    virtual ~NarrowPhrase() = default;

//  [[script::bindings::auto]]
    virtual Box makeAABBShape(const Rect& aabb) = 0;
//  [[script::bindings::auto]]
    virtual Box makeBallShape(const V2& position, float radius) = 0;
//  [[script::bindings::auto]]
    virtual Box makeBoxShape(const Rect& bounds) = 0;
//  [[script::bindings::auto]]
    virtual Box makeCapsuleShape(const V2& p1, const V2& p2, float radius) = 0;
//  [[script::bindings::auto]]
    virtual Box makePolygonShape(const std::vector<V2>& vertices) = 0;

    virtual Ray toRay(const V2& from, const V2& to) = 0;

    virtual bool collisionManifold(const BroadPhrase::Candidate& candidateOne, const BroadPhrase::Candidate& candidateOther, CollisionManifold& collisionManifold) = 0;
    virtual bool rayCastManifold(const Ray& ray, const BroadPhrase::Candidate& candidate, RayCastManifold& rayCastManifold) = 0;
};

}

#endif
