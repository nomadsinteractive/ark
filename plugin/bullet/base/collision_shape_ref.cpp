#include "bullet/base/collision_shape_ref.h"

namespace ark::plugin::bullet {

CollisionShapeRef::CollisionShapeRef(sp<btCollisionShape> shape, const btScalar mass)
    : _shape(std::move(shape)), _mass(mass)
{
}

btCollisionShape* CollisionShapeRef::btShape() const
{
    return _shape.get();
}

btScalar CollisionShapeRef::mass() const
{
    return _mass;
}

void CollisionShapeRef::setMass(const btScalar mass)
{
    _mass = mass;
}

}
