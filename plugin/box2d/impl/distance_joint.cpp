#include "box2d/impl/distance_joint.h"

#include "box2d/impl/body.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/world.h"

namespace ark {
namespace plugin {
namespace box2d {

DistanceJoint::DistanceJoint(const sp<World>& world, const Body& b1, const Body& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected, float frequencyHz, float dampingRatio)
    : Joint(world, makeJoint(world->world(), b1, b2, anchorA, anchorB, length, collideConnected, frequencyHz, dampingRatio))
{
}

b2Joint*DistanceJoint::makeJoint(b2World& b2World, const Body& b1, const Body& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected, float frequencyHz, float dampingRatio)
{
    b2DistanceJointDef jointDef;
    jointDef.bodyA = b1.body();
    jointDef.bodyB = b2.body();
    jointDef.localAnchorA = b2Vec2(anchorA.x(), anchorA.y());
    jointDef.localAnchorB = b2Vec2(anchorB.x(), anchorB.y());
    jointDef.collideConnected = collideConnected;
    jointDef.frequencyHz = frequencyHz;
    jointDef.dampingRatio = dampingRatio;
    jointDef.length = length;
    return b2World.CreateJoint(&jointDef);
}

}
}
}
