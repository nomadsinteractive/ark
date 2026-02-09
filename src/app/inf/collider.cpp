#include "app/inf/collider.h"

#include "core/types/ref.h"

#include "graphics/base/shape.h"

namespace ark {

sp<Rigidbody> Collider::createBody(sp<Collider> self, Rigidbody::BodyType bodyType, sp<Shape> shape, sp<Vec3> position, sp<Vec4> rotation, sp<CollisionFilter> collisionFilter, sp<Boolean> discarded)
{
    if(!shape)
        shape = sp<Shape>::make();
    if(shape->type().hash() == Shape::TYPE_NONE)
        return sp<Rigidbody>::make(Rigidbody::Impl{sp<Rigidbody::Stub>::make(sp<Ref>::make(0, nullptr, std::move(discarded)), bodyType, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter)), std::move(self), nullptr}, true);

    Rigidbody::Impl impl = self->createBody(bodyType, std::move(shape), std::move(position), std::move(rotation), std::move(collisionFilter), std::move(discarded));
    impl._collider = std::move(self);
    return sp<Rigidbody>::make(std::move(impl), false);
}

}