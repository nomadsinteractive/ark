#include "bullet/base/collision_shape.h"

namespace ark::plugin::bullet {

CollisionShape::CollisionShape(sp<btCollisionShape> shape, const btScalar mass)
    : _shape(std::move(shape)), _mass(mass)
{
}

const sp<btCollisionShape>& CollisionShape::btShape() const
{
    return _shape;
}

btScalar CollisionShape::mass() const
{
    return _mass;
}

void CollisionShape::setMass(const btScalar mass)
{
    _mass = mass;
}

}
