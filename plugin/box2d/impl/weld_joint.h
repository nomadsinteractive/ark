#ifndef ARK_PLUGIN_BOX2D_IMPL_WELD_JOINT_H_
#define ARK_PLUGIN_BOX2D_IMPL_WELD_JOINT_H_

#include <Box2D/Box2D.h>

#include "core/base/object.h"
#include "core/types/implements.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"
#include "box2d/impl/joint.h"

namespace ark {
namespace plugin {
namespace box2d {

class ARK_PLUGIN_BOX2D_API WeldJoint : public Object, public Joint, Implements<Object> {
public:
//  [[script::bindings::auto]]
    WeldJoint(const sp<ColliderBox2D>& world, const RigidBodyBox2D& b1, const RigidBodyBox2D& b2, const V2& anchorA, const V2& anchorB, float referenceAngle, bool collideConnected = false);

/*
//  [[script::bindings::auto]]
    void release();
//  [[script::bindings::auto]]
    void dispose();
*/

private:
    static b2Joint* makeJoint(b2World& b2World, const RigidBodyBox2D& b1, const RigidBodyBox2D& b2, const V2& anchorA, const V2& anchorB, float referenceAngle, bool collideConnected);

};

}
}
}

#endif
