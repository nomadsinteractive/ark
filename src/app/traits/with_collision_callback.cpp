#include "app/traits/with_collision_callback.h"

#include "core/base/bean_factory.h"
#include "core/util/holder_util.h"

namespace ark {

WithCollisionCallback::WithCollisionCallback(sp<CollisionCallback> collisionCallback)
    : _collision_callback(std::move(collisionCallback))
{
}

TypeId WithCollisionCallback::onPoll(WiringContext& /*context*/)
{
    return Type<CollisionCallback>::id();
}

void WithCollisionCallback::onWire(const WiringContext& /*context*/)
{
}

void WithCollisionCallback::onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold)
{
    return _collision_callback->onBeginContact(rigidBody, manifold);
}

void WithCollisionCallback::onEndContact(const RigidBody& rigidBody)
{
    _collision_callback->onEndContact(rigidBody);
}

void WithCollisionCallback::traverse(const Visitor& visitor)
{
    HolderUtil::visit(_collision_callback, visitor);
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
