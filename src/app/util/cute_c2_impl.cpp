#include "app/util/cute_c2_impl.h"

#define CUTE_C2_IMPLEMENTATION
#include <cute_c2.h>

#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/quaternion.h"
#include "graphics/base/transform.h"
#include "graphics/base/v2.h"

namespace ark {

C2Shape::C2Shape()
{
    memset(this, 0, sizeof(*this));
}

C2RigidBody::C2RigidBody(const sp<Vec3>& position, const sp<Quaternion>& rotate, bool isStaticBody)
    : _position(position), _rotate(rotate), _is_static_body(isStaticBody)
{
}

void C2RigidBody::makeAABB(const Rect& aabb)
{
    C2Shape shape;
    shape.t = C2_TYPE_AABB;
    shape.s.aabb.min.x = aabb.left();
    shape.s.aabb.min.y = aabb.top();
    shape.s.aabb.max.x = aabb.right();
    shape.s.aabb.max.y = aabb.bottom();
    _shapes.push_back(shape);
}

void C2RigidBody::makeCircle(const V2& p, float radius)
{
    C2Shape shape;
    shape.t = C2_TYPE_CIRCLE;
    shape.s.circle.p.x = p.x();
    shape.s.circle.p.y = p.y();
    shape.s.circle.r = radius;
    _shapes.push_back(shape);
}

void C2RigidBody::makeCapsule(const V2& p1, const V2& p2, float radius)
{
    C2Shape shape;
    shape.t = C2_TYPE_CAPSULE;
    shape.s.capsule.a.x = p1.x();
    shape.s.capsule.a.y = p1.y();
    shape.s.capsule.b.x = p2.x();
    shape.s.capsule.b.y = p2.y();
    shape.s.capsule.r = radius;
    _shapes.push_back(shape);
}

void C2RigidBody::makePoly(const c2Poly& poly)
{
    C2Shape shape;
    shape.t = C2_TYPE_POLY;
    shape.s.poly = poly;
    c2MakePoly(&shape.s.poly);
    _shapes.push_back(shape);
}

void C2RigidBody::setShapes(const std::vector<C2Shape>& shapes, const V2& scale)
{
    _shapes = shapes;
    for(C2Shape& i: _shapes)
        for(int32_t j = 0; j < i.s.poly.count; ++j)
        {
            i.s.poly.verts[j].x *= scale.x();
            i.s.poly.verts[j].y *= scale.y();
        }
}

void C2RigidBody::collideManifold(const C2RigidBody& other, c2Manifold* m) const
{
    c2x x1, x2;
    std::vector<C2Shape> s1 = transform(x1);
    std::vector<C2Shape> s2 = other.transform(x2);
    for(const C2Shape& i : s1)
        for(const C2Shape& j : s2)
        {
            c2Collide(&i.s, &x1, i.t, &j.s, &x2, j.t, m);
            if(m->count > 0)
                return;
        }
}

bool C2RigidBody::isStaticBody() const
{
    return _is_static_body;
}

std::vector<C2Shape> C2RigidBody::transform(c2x& x) const
{
    float rotation = _rotate ? _rotate->rotation() : 0;

    const V2 pos = _position->val();
    x.p.x = pos.x();
    x.p.y = pos.y();
    x.r.c = Math::cos(rotation);
    x.r.s = Math::sin(rotation);

    if(_is_static_body)
        return _shapes;

    std::vector<C2Shape> shapes;
    for(const C2Shape& i : _shapes) {
        C2Shape shape;
        shape.t = i.t;
        switch(i.t)
        {
        case C2_TYPE_CIRCLE:
            shape.s.circle.r = i.s.circle.r;
            translate(i.s.circle.p.x, i.s.circle.p.y, pos, shape.s.circle.p.x, shape.s.circle.p.y);
            break;
        case C2_TYPE_AABB:
            DCHECK(rotation == 0, "Rotation: %.2f, which is not supported on AABBs", rotation);
            translate(i.s.aabb.min.x, i.s.aabb.min.y, pos, shape.s.aabb.min.x, shape.s.aabb.min.y);
            translate(i.s.aabb.max.x, i.s.aabb.max.y, pos, shape.s.aabb.max.x, shape.s.aabb.max.y);
            break;
        case C2_TYPE_CAPSULE:
            shape.s.capsule.r = i.s.capsule.r;
            translate(i.s.capsule.a.x, i.s.capsule.a.y, pos, shape.s.capsule.a.x, shape.s.capsule.a.y);
            translate(i.s.capsule.b.x, i.s.capsule.b.y, pos, shape.s.capsule.b.x, shape.s.capsule.b.y);
            break;
        case C2_TYPE_POLY:
            shape.s.poly = i.s.poly;
            break;
        }
        shapes.push_back(shape);
    }
    return shapes;
}

void C2RigidBody::translate(float x, float y, const V2& translate, float& ox, float oy) const
{
    ox = x + translate.x();
    oy = y + translate.y();
}

}
