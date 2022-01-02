#ifndef ARK_APP_UTIL_SHAPE_CUTE_C2_IMPL_H_
#define ARK_APP_UTIL_SHAPE_CUTE_C2_IMPL_H_

#include <vector>

#include <cute_c2.h>

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ShapeCuteC2 {
public:
    ShapeCuteC2();
    ShapeCuteC2(const ShapeCuteC2& other, const V2& translate, float rotation);

    bool collideManifold(const ShapeCuteC2& other, CollisionManifold& collisionManifold) const;
    bool rayCastManifold(const c2Ray& ray, RayCastManifold& rayCastManifold) const;

    C2_TYPE t;
    union {
        c2AABB aabb;
        c2Capsule capsule;
        c2Circle circle;
        c2Poly poly;
    } s;
    c2x x;

private:
    void transform(const V2& position, float rotation);
};

}

#endif
