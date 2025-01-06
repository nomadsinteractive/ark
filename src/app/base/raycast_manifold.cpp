#include "app/base/raycast_manifold.h"

namespace ark {

RayCastManifold::RayCastManifold(float distance, const V3& normal, sp<Rigidbody> rigidbody)
    : _distance(distance), _normal(normal), _rigidbody(std::move(rigidbody))
{
}

float RayCastManifold::distance() const
{
    return _distance;
}

const V3& RayCastManifold::normal() const
{
    return _normal;
}

const sp<Rigidbody>& RayCastManifold::rigidbody() const
{
    return _rigidbody;
}

void RayCastManifold::setRigidBody(sp<Rigidbody> rigidbody)
{
    _rigidbody = std::move(rigidbody);
}

}
