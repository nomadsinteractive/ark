#include "box2d/impl/weld_joint.h"

#include "plugin/box2d/impl/body.h"
#include "plugin/box2d/impl/world.h"

namespace ark {
namespace plugin {
namespace box2d {

WeldJoint::WeldJoint(const sp<World>& world, const Body& b1, const Body& b2, const V2& anchorA, const V2& anchorB, float referenceAngle, bool collideConnected, float frequencyHz, float dampingRatio)
    : _world(world)
{
    b2WeldJointDef jointDef;
    jointDef.bodyA = b1.body();
    jointDef.bodyB = b2.body();
    jointDef.localAnchorA = b2Vec2(anchorA.x(), anchorA.y());
    jointDef.localAnchorB = b2Vec2(anchorB.x(), anchorB.y());
    jointDef.referenceAngle = referenceAngle;
    jointDef.collideConnected = collideConnected;
    jointDef.frequencyHz = frequencyHz;
    jointDef.dampingRatio = dampingRatio;
    _joint = static_cast<b2WeldJoint*>(world->world().CreateJoint(&jointDef));
}

WeldJoint::~WeldJoint()
{
    _world->world().DestroyJoint(_joint);
}

}
}
}
