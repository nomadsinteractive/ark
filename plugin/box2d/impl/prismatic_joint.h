#ifndef ARK_PLUGIN_BOX2D_IMPL_PRISMATIC_JOINT_H_
#define ARK_PLUGIN_BOX2D_IMPL_PRISMATIC_JOINT_H_

#include <Box2D/Box2D.h>

#include "core/base/object.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/api.h"
#include "box2d/forwarding.h"

namespace ark {
namespace plugin {
namespace box2d {

class ARK_PLUGIN_BOX2D_API PrismaticJoint : public Object, Implements<Object> {
public:
//  [[script::bindings::auto]]
    PrismaticJoint(const sp<World>& world, const Body& b1, const Body& b2, const V2& anchorA, const V2& anchorB, float referenceAngle, bool collideConnected = false);

private:
    sp<Joint> _joint;

};

}
}
}

#endif
