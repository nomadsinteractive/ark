#include "app/base/collision_manifold.h"

namespace ark {

CollisionManifold::CollisionManifold(const V3& normal)
    : _normal(normal)
{
}

const V3& CollisionManifold::normal() const
{
    return _normal;
}

}
