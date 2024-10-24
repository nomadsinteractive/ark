#include "app/traits/with_collision_callback.h"

#include "core/base/bean_factory.h"
#include "core/traits/with_debris.h"

namespace ark {

WithCollisionCallback::WithCollisionCallback(sp<CollisionCallback> collisionCallback)
    : _collision_callback(std::move(collisionCallback))
{
}

TypeId WithCollisionCallback::onPoll(WiringContext& /*context*/)
{
    return Type<CollisionCallback>::id();
}

void WithCollisionCallback::onWire(const WiringContext& context)
{
    if(sp<Debris> debris = _collision_callback.tryCast<Debris>())
        WithDebris::ensureComponent(context)->track(std::move(debris));
}

void WithCollisionCallback::onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold)
{
    return _collision_callback->onBeginContact(rigidBody, manifold);
}

void WithCollisionCallback::onEndContact(const RigidBody& rigidBody)
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
