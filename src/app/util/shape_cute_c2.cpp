#include "app/util/shape_cute_c2.h"

#define CUTE_C2_IMPLEMENTATION
#include <cute_c2.h>

#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/transform.h"
#include "graphics/base/v2.h"

#include "app/base/collision_manifold.h"
#include "app/base/raycast_manifold.h"

namespace ark {

static void translate(float x, float y, const V2& translate, float& ox, float& oy)
{
    ox = x + translate.x();
    oy = y + translate.y();
}

ShapeCuteC2::ShapeCuteC2()
    : width(0), height(0), t(C2_TYPE_NONE)
{
    memset(&s, 0, sizeof(s));
    memset(&x, 0, sizeof(x));
}

ShapeCuteC2::ShapeCuteC2(const ShapeCuteC2& other, const V2& translate, float rotation)
    : t(other.t), s(other.s), _collision_filter(other._collision_filter)
{
    doTransform(translate, rotation);
}

bool ShapeCuteC2::collideManifold(const ShapeCuteC2& other, CollisionManifold& collisionManifold) const
{
    c2Manifold m;
    c2Collide(&s, &x, t, &other.s, &other.x, other.t, &m);
    if(m.count > 0)
    {
        const c2v& contactPoint = m.contact_points[0];
        collisionManifold = CollisionManifold(V3(contactPoint.x, contactPoint.y, 0), V3(m.n.x, m.n.y, 0));
        return true;
    }
    return false;
}

bool ShapeCuteC2::rayCastManifold(const c2Ray& ray, RayCastManifold& rayCastManifold) const
{
    c2Raycast raycast;
    bool r = c2CastRay(ray, &s, &x, t, &raycast);
    if(r)
        rayCastManifold = RayCastManifold(raycast.t, V3(raycast.n.x, raycast.n.y, 0), rayCastManifold.rigidBody());
    return r;
}

ShapeCuteC2 ShapeCuteC2::transform(const V2& position, float rotation) const
{
    return ShapeCuteC2(*this, position, rotation);
}

void ShapeCuteC2::resize(const V2& scale)
{
    switch(t)
    {
    case C2_TYPE_CIRCLE:
        s.circle.p.x *= scale.x();
        s.circle.p.y *= scale.y();
        s.circle.r *= scale.x();
        break;
    case C2_TYPE_AABB:
        s.aabb.min.x *= scale.x();
        s.aabb.max.x *= scale.x();
        s.aabb.min.y *= scale.y();
        s.aabb.max.y *= scale.y();
        break;
    case C2_TYPE_CAPSULE:
        s.capsule.a.x *= scale.x();
        s.capsule.b.x *= scale.x();
        s.capsule.a.y *= scale.y();
        s.capsule.b.y *= scale.y();
        s.capsule.r *= scale.x();
        break;
    case C2_TYPE_POLY:
        for(int32_t i = 0; i < s.poly.count; ++i)
        {
            s.poly.verts[i].x *= scale.x();
            s.poly.verts[i].y *= scale.y();
        }
        break;
    case C2_TYPE_NONE:
        break;
    }
}

void ShapeCuteC2::doTransform(const V2& position, float rotation)
{
    switch(t)
    {
    case C2_TYPE_CIRCLE:
        translate(s.circle.p.x, s.circle.p.y, position, s.circle.p.x, s.circle.p.y);
        break;
    case C2_TYPE_AABB:
        DCHECK(rotation == 0, "Rotation: %.2f, which is not supported on AABBs", rotation);
        translate(s.aabb.min.x, s.aabb.min.y, position, s.aabb.min.x, s.aabb.min.y);
        translate(s.aabb.max.x, s.aabb.max.y, position, s.aabb.max.x, s.aabb.max.y);
        break;
    case C2_TYPE_CAPSULE:
        translate(s.capsule.a.x, s.capsule.a.y, position, s.capsule.a.x, s.capsule.a.y);
        translate(s.capsule.b.x, s.capsule.b.y, position, s.capsule.b.x, s.capsule.b.y);
        break;
    case C2_TYPE_POLY:
        x.p.x = position.x();
        x.p.y = position.y();
        x.r.c = Math::cos(rotation);
        x.r.s = Math::sin(rotation);
        break;
    case C2_TYPE_NONE:
        break;
    }
}

}
