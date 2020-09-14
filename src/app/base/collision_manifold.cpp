#include "app/base/collision_manifold.h"

namespace ark {

CollisionManifold::CollisionManifold(const V3& contactPoint, const V3& normal)
    : _contact_point(contactPoint), _normal(normal)
{
}

const V3& CollisionManifold::contactPoint() const
{
    return _contact_point;
}

const V3& CollisionManifold::normal() const
{
    return _normal;
}

}
