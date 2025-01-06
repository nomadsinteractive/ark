#pragma once

#include <vector>

#include <cute_c2.h>

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/base/collision_filter.h"

namespace ark {

class ShapeCuteC2 {
public:
    ShapeCuteC2();
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(ShapeCuteC2);

    bool collideManifold(const ShapeCuteC2& other, CollisionManifold& collisionManifold) const;
    Optional<RayCastManifold> rayCastManifold(const c2Ray& ray) const;

    ShapeCuteC2 transform(const V2& position, const V4& quaternion) const;

    C2_TYPE t;
    union {
        c2AABB aabb;
        c2Capsule capsule;
        c2Circle circle;
        c2Poly poly;
    } s;
    c2x x;

    CollisionFilter _collision_filter;

    void resize(const V2& scale);

private:
    ShapeCuteC2(const ShapeCuteC2& other, const V2& translate, const V4& rotation);

    void doTransform(const V2& position, const V4& rotation);
};

}
