#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API BroadPhraseCallback {
public:

    virtual ~BroadPhraseCallback() = default;

//  [[script::bindings::interface]]
    virtual void onRigidbodyCandidate(RefId rigidbodyId) = 0;
//  [[script::bindings::interface]]
    virtual void onStaticCandidate(RefId candidateId, V3 position, V4 quaternion, sp<Shape> shape, sp<CollisionFilter> collisionFilter) = 0;
};

}
