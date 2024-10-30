#include "bullet/base/collision_shape.h"

namespace ark::plugin::bullet {

CollisionShape::CollisionShape(btCollisionShape* shape, btScalar mass)
    : _shape(shape), _mass(mass)
{
}

btCollisionShape* CollisionShape::btShape() const
{
    return _shape.get();
}

btScalar CollisionShape::mass() const
{
    return _mass;
}

void CollisionShape::setMass(btScalar mass)
{
    _mass = mass;
}

}
