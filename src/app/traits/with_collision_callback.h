#pragma once

#include "core/base/api.h"
#include "core/inf/holder.h"
#include "core/inf/wirable.h"

#include "app/inf/collision_callback.h"

namespace ark {

class ARK_API WithCollisionCallback : public Wirable, public CollisionCallback, public Holder {
public:
//  [[script::bindings::auto]]
    WithCollisionCallback(sp<CollisionCallback> collisionCallback);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) override;
    void onEndContact(const RigidBody& rigidBody) override;

    void traverse(const Visitor& visitor) override;

//  [[plugin::builder("with_collision_callback")]]
    class BUILDER : public Builder<Wirable> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        sp<Builder<CollisionCallback>> _collision_callback;
    };

private:
    sp<CollisionCallback> _collision_callback;
};

}
