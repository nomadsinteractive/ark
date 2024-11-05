#pragma once

#include "core/base/api.h"
#include "core/traits/with_debris.h"
#include "core/inf/wirable.h"

#include "app/inf/collision_callback.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API WithCollisionCallback final : public Wirable, public Debris, public CollisionCallback {
public:
//  [[script::bindings::auto]]
    WithCollisionCallback(sp<CollisionCallback> collisionCallback);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;
    void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    const sp<CollisionCallback>& collisionCallback() const;
//  [[script::bindings::property]]
    void setCollisionCallback(sp<CollisionCallback> collisionCallback);

    void onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) override;
    void onEndContact(const Rigidbody& rigidBody) override;

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

    WithDebris _with_debris;
};

}
