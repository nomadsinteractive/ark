#pragma once

#include <Box2D/Box2D.h>

#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/joint.h"

namespace ark::plugin::box2d {

class ARK_PLUGIN_BOX2D_API DistanceJoint : public Joint {
public:
//  [[script::bindings::auto]]
    DistanceJoint(const sp<ColliderBox2D>& world, const RigidbodyBox2D& b1, const RigidbodyBox2D& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected = false);

/*
//  [[script::bindings::auto]]
    void release();
//  [[script::bindings::auto]]
    void dispose();
*/

private:
    static b2JointId makeJoint(b2WorldId b2World, const RigidbodyBox2D& b1, const RigidbodyBox2D& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected);
};

}
