#pragma once

#include "core/base/api.h"
#include "core/inf/wirable.h"

#include "app/inf/collision_callback.h"

namespace ark {

class ARK_API WithCollisionCallback final : public Wirable, public CollisionCallback {
public:
//  [[script::bindings::auto]]
    WithCollisionCallback(sp<CollisionCallback> collisionCallback);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) override;
    void onEndContact(const RigidBody& rigidBody) override;

//  [[plugin::builder("with_collision_callback")]]
    class BUILDER final : public Builder<Wirable> {
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
