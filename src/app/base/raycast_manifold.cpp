#include "app/base/raycast_manifold.h"

namespace ark {

RayCastManifold::RayCastManifold(float distance, const V3& normal, sp<RigidBody> rigidBody)
    : _distance(distance), _normal(normal), _rigid_body(std::move(rigidBody))
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

const sp<RigidBody>& RayCastManifold::rigidBody() const
{
    return _rigid_body;
}

}
