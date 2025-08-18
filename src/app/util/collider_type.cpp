#include "app/util/collider_type.h"

#include "core/types/ref.h"

#include "graphics/components/shape.h"
#include "app/inf/collider.h"

namespace ark {

sp<Rigidbody> ColliderType::createBody(const sp<Collider>& self, Rigidbody::BodyType bodyType, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
{
    if(!shape)
        shape = sp<Shape>::make();
    if(shape->type().hash() == Shape::TYPE_NONE)
        return sp<Rigidbody>::make(Rigidbody::Impl{sp<Rigidbody::Stub>::make(sp<Ref>::make(0, nullptr, std::move(discarded)), bodyType, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter)), self, nullptr}, true);

    Rigidbody::Impl impl = self->createBody(bodyType, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter), std::move(discarded));
    return sp<Rigidbody>::make(std::move(impl), false);
}

sp<Shape> ColliderType::createShape(const sp<Collider>& self, const NamedHash& type, sp<Vec3> size, sp<Vec3> origin)
{
    return self->createShape(type, std::move(size), std::move(origin));
}

Vector<RayCastManifold> ColliderType::rayCast(const sp<Collider>& self, const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter)
{
    return self->rayCast(from, to, collisionFilter);
}

}
