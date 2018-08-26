#ifndef ARK_PLUGIN_BOX2D_IMPL_DISTANCE_JOINT_H_
#define ARK_PLUGIN_BOX2D_IMPL_DISTANCE_JOINT_H_

#include <Box2D/Box2D.h>

#include "core/base/object.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "box2d/api.h"

namespace ark {
namespace plugin {
namespace box2d {

class World;
class Body;

class ARK_PLUGIN_BOX2D_API DistanceJoint : public Object, Implements<Object> {
public:
//  [[script::bindings::auto]]
    DistanceJoint(const sp<World>& world, const Body& b1, const Body& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected = false, float frequencyHz = 0, float dampingRatio = 0);
    ~DistanceJoint();

private:
    sp<World> _world;
    b2DistanceJoint* _joint;

};

}
}
}

#endif
