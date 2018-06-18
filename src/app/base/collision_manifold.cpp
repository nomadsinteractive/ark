#include "app/base/collision_manifold.h"

namespace ark {

CollisionManifold::CollisionManifold(const V& normal)
    : _normal(normal)
{
}

const V& CollisionManifold::normal() const
{
    return _normal;
}

}
