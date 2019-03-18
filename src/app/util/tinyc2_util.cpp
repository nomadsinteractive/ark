#include "app/util/tinyc2_util.h"

#define TINYC2_IMPLEMENTATION
#include <tinyc2.h>

#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/log.h"

#include "graphics/base/rect.h"
#include "graphics/base/rotate.h"
#include "graphics/base/transform.h"
#include "graphics/base/v2.h"

namespace ark {

C2RigidBody::C2RigidBody(const sp<Vec2>& position, const sp<Rotate>& rotate, bool isStaticBody)
    : _type(C2_AABB), _position(position), _rotate(rotate), _is_static_body(isStaticBody)
{
    memset(&_shape, 0, sizeof(_shape));
}

void C2RigidBody::makeAABB(const Rect& aabb)
{
    _type = C2_AABB;
    _shape.aabb.min.x = aabb.left();
    _shape.aabb.min.y = aabb.top();
    _shape.aabb.max.x = aabb.right();
    _shape.aabb.max.y = aabb.bottom();
}

void C2RigidBody::makeCircle(const V2& p, float radius)
{
    _type = C2_CIRCLE;
    _shape.circle.p.x = p.x();
    _shape.circle.p.y = p.y();
    _shape.circle.r = radius;
}

void C2RigidBody::makeCapsule(const V2& p1, const V2& p2, float radius)
{
    _type = C2_CAPSULE;
    _shape.capsule.a.x = p1.x();
    _shape.capsule.a.y = p1.y();
    _shape.capsule.b.x = p2.x();
    _shape.capsule.b.y = p2.y();
    _shape.capsule.r = radius;
}

void C2RigidBody::makePoly(const c2Poly& poly)
{
    _type = C2_POLY;
    _shape.poly = poly;
    c2MakePoly(&_shape.poly);
}

void C2RigidBody::makeShape(C2_TYPE type, const C2Shape& shape)
{
    _type = type;
    _shape = shape;
}

int C2RigidBody::collide(const C2RigidBody& other) const
{
    C2Shape s1, s2;
    c2x x1, x2;
    const C2Shape& us1 = updateShape(s1, x1);
    const C2Shape& us2 = other.updateShape(s2, x2);
    return c2Collided(&us1, &x1, _type, &us2, &x2, other._type);
}

void C2RigidBody::collideManifold(const C2RigidBody& other, c2Manifold* m) const
{
    C2Shape s1, s2;
    c2x x1, x2;
    const C2Shape& us1 = updateShape(s1, x1);
    const C2Shape& us2 = other.updateShape(s2, x2);
    c2Collide(&us1, &x1, _type, &us2, &x2, other._type, m);
}

const C2Shape& C2RigidBody::shape() const
{
    return _shape;
}

C2Shape& C2RigidBody::shape()
{
    return _shape;
}

const sp<Rotate>& C2RigidBody::rotate() const
{
    return _rotate;
}

bool C2RigidBody::isStaticBody() const
{
    return _is_static_body;
}

const C2Shape& C2RigidBody::updateShape(C2Shape& shape, c2x& x) const
{
    if(_is_static_body)
    {
        x = c2xIdentity();
        return _shape;
    }

    const V2 pos = _position->val();
    Transform::Snapshot ts;
    ts.rotate_value = _rotate ? _rotate->radians() : 0;
    ts.rotate_direction = _rotate ? _rotate->direction()->val() : V3();
    switch(_type)
    {
    case C2_CIRCLE:
        shape.circle.r = _shape.circle.r;
        ts.map(_shape.circle.p.x, _shape.circle.p.y, pos.x(), pos.y(), shape.circle.p.x, shape.circle.p.y);
        break;
    case C2_AABB:
        DCHECK(ts.rotate_value == 0, "Rotation: %.2f, which is not supported on AABBs", ts.rotate_value);
        ts.map(_shape.aabb.min.x, _shape.aabb.min.y, pos.x(), pos.y(), shape.aabb.min.x, shape.aabb.min.y);
        ts.map(_shape.aabb.max.x, _shape.aabb.max.y, pos.x(), pos.y(), shape.aabb.max.x, shape.aabb.max.y);
        break;
    case C2_CAPSULE:
        shape.capsule.r = _shape.capsule.r;
        ts.map(_shape.capsule.a.x, _shape.capsule.a.y, pos.x(), pos.y(), shape.capsule.a.x, shape.capsule.a.y);
        ts.map(_shape.capsule.b.x, _shape.capsule.b.y, pos.x(), pos.y(), shape.capsule.b.x, shape.capsule.b.y);
        break;
    case C2_POLY:
        DWARN(ts.rotate_direction == Rotate::Z_AXIS, "Rotate Direction: (%.2f, %.2f, %.2f). Tinyc2 Only supports z-axis(2D) rotation", ts.rotate_direction.x(), ts.rotate_direction.y(), ts.rotate_direction.z());
        shape.poly = _shape.poly;
        x.p.x = pos.x();
        x.p.y = pos.y();
        x.r.c = Math::cos(ts.rotate_value);
        x.r.s = Math::sin(ts.rotate_value);
        break;
    }
    return shape;
}

}
