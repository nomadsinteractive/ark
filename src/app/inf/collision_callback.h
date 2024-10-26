#pragma once

#include "core/base/api.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API CollisionCallback {
public:
    virtual ~CollisionCallback() = default;

    virtual void onBeginContact(const Rigidbody& rigidBody, const CollisionManifold& manifold) = 0;
    virtual void onEndContact(const Rigidbody& rigidBody) = 0;
};

}
