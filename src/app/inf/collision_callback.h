#pragma once

#include "core/base/api.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API CollisionCallback {
public:
    virtual ~CollisionCallback() = default;

    virtual void onBeginContact(const RigidBody& rigidBody, const CollisionManifold& manifold) = 0;
    virtual void onEndContact(const RigidBody& rigidBody) = 0;
};

}
