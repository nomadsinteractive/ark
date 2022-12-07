#include "box2d/impl/distance_joint.h"

#include "box2d/impl/rigid_body_box2d.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/collider_box2d.h"

namespace ark {
namespace plugin {
namespace box2d {

DistanceJoint::DistanceJoint(const sp<ColliderBox2D>& world, const RigidBodyBox2D& b1, const RigidBodyBox2D& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected)
    : Joint(world, makeJoint(world->world(), b1, b2, anchorA, anchorB, length, collideConnected))
{
}

b2Joint*DistanceJoint::makeJoint(b2World& b2World, const RigidBodyBox2D& b1, const RigidBodyBox2D& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected)
{
    b2DistanceJointDef jointDef;
    jointDef.bodyA = b1.body();
    jointDef.bodyB = b2.body();
    jointDef.localAnchorA = b2Vec2(anchorA.x(), anchorA.y());
    jointDef.localAnchorB = b2Vec2(anchorB.x(), anchorB.y());
    jointDef.collideConnected = collideConnected;
    jointDef.length = length;
    return b2World.CreateJoint(&jointDef);
}

}
}
}
