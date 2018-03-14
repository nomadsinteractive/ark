#ifndef ARK_APP_UTIL_TINYC2_UTIL_H_
#define ARK_APP_UTIL_TINYC2_UTIL_H_

#include <tinyc2.h>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

union C2Shape {
    c2AABB aabb;
    c2Capsule capsule;
    c2Circle circle;
    c2Poly poly;
};

class C2RigidBody {
public:
    C2RigidBody(const sp<VV2>& position, const sp<Transform>& transform, bool isStaticBody);
    C2RigidBody(const C2RigidBody& other) = default;

    void makeAABB(const Rect& aabb);
    void makeCircle(const V2& p, float radius);
    void makeCapsule(const V2& p1, const V2& p2, float radius);
    void makePoly(const c2Poly& poly);
    void makeShape(C2_TYPE type, const C2Shape& shape);

    int collide(const C2RigidBody& other) const;
    void collideManifold(const C2RigidBody& other, c2Manifold* m) const;

    const C2Shape& shape() const;
    C2Shape& shape();

    const sp<Transform>& transform() const;

    bool isStaticBody() const;

private:
    const C2Shape& updateShape(C2Shape& shape, c2x& x) const;

private:
    C2_TYPE _type;
    C2Shape _shape;
    sp<VV2> _position;
    sp<Transform> _transform;
    bool _is_static_body;
};

}

#endif
