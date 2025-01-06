#include "app/util/collider_type.h"

#include "app/inf/collider.h"

namespace ark {

sp<Rigidbody> ColliderType::createBody(const sp<Collider>& self, Rigidbody::BodyType bodyType, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<Boolean> discarded)
{
    Rigidbody::Impl impl = self->createBody(bodyType, std::move(shape), std::move(position), std::move(rotation), std::move(discarded));
    return sp<Rigidbody>::make(std::move(impl), false);
}

sp<Shape> ColliderType::createShape(const sp<Collider>& self, const NamedHash& type, sp<Vec3> size, sp<Vec3> origin)
{
    return self->createShape(type, std::move(size), std::move(origin));
}

std::vector<RayCastManifold> ColliderType::rayCast(const sp<Collider>& self, const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter)
{
    return self->rayCast(from, to, collisionFilter);
}

}
