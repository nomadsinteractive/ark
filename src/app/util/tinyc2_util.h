#ifndef ARK_APP_UTIL_TINYC2_UTIL_H_
#define ARK_APP_UTIL_TINYC2_UTIL_H_

#include <vector>

#include <tinyc2.h>

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

struct C2Shape {
    C2_TYPE t;

    union {
        c2AABB aabb;
        c2Capsule capsule;
        c2Circle circle;
        c2Poly poly;
    } s;
};

class C2RigidBody {
public:
    C2RigidBody(const sp<Vec2>& position, const sp<Rotate>& rotate, bool isStaticBody);
    C2RigidBody(const C2RigidBody& other) = default;

    void makeAABB(const Rect& aabb);
    void makeCircle(const V2& p, float radius);
    void makeCapsule(const V2& p1, const V2& p2, float radius);
    void makePoly(const c2Poly& poly);
    void setShapes(const std::vector<C2Shape>& shapes, const V2& scale);

    void collideManifold(const C2RigidBody& other, c2Manifold* m) const;
    bool isStaticBody() const;

private:
    std::vector<C2Shape> transform(c2x& x) const;

private:
    std::vector<C2Shape> _shapes;
    sp<Vec2> _position;
    sp<Rotate> _rotate;
    bool _is_static_body;
};

}

#endif
