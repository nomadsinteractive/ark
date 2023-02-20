#ifndef ARK_APP_INF_COLLISION_CALLBACK_H_
#define ARK_APP_INF_COLLISION_CALLBACK_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API CollisionCallback {
public:
    virtual ~CollisionCallback() = default;

    virtual void onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold) = 0;
    virtual void onEndContact(const sp<RigidBody>& rigidBody) = 0;
};

}

#endif
