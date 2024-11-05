#include "app/traits/with_collision_callback.h"

#include "core/base/bean_factory.h"
#include "core/traits/with_debris.h"

namespace ark {

WithCollisionCallback::WithCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    setCollisionCallback(std::move(collisionCallback));
}

TypeId WithCollisionCallback::onPoll(WiringContext& /*context*/)
{
    return Type<CollisionCallback>::id();
}

void WithCollisionCallback::onWire(const WiringContext& context)
{
    _with_debris.onWire(context);
}

void WithCollisionCallback::traverse(const Visitor& visitor)
{
    _with_debris.traverse(visitor);
}

const sp<CollisionCallback>& WithCollisionCallback::collisionCallback() const
{
    return _collision_callback;
}

void WithCollisionCallback::setCollisionCallback(sp<CollisionCallback> collisionCallback)
{
    if(const sp<Debris> debris = collisionCallback.tryCast<Debris>())
        _with_debris.track(debris);
    _collision_callback = std::move(collisionCallback);
}

void WithCollisionCallback::onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold)
{
    return _collision_callback->onBeginContact(rigidBody, manifold);
}

void WithCollisionCallback::onEndContact(const Rigidbody& rigidBody)
{
    _collision_callback->onEndContact(rigidBody);
}

WithCollisionCallback::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _collision_callback(factory.ensureBuilder<CollisionCallback>(manifest, constants::COLLISION_CALLBACK))
{
}

sp<Wirable> WithCollisionCallback::BUILDER::build(const Scope& args)
{
    return sp<Wirable>::make<WithCollisionCallback>(_collision_callback->build(args));
}

}
