#include "box2d/impl/distance_joint.h"

#include "box2d/impl/rigidbody_box2d.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/collider_box2d.h"

namespace ark::plugin::box2d {

DistanceJoint::DistanceJoint(const sp<ColliderBox2D>& world, const RigidbodyBox2D& b1, const RigidbodyBox2D& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected)
    : Joint(world, makeJoint(world->world(), b1, b2, anchorA, anchorB, length, collideConnected))
{
}

b2JointId DistanceJoint::makeJoint(b2WorldId b2World, const RigidbodyBox2D& b1, const RigidbodyBox2D& b2, const V2& anchorA, const V2& anchorB, float length, bool collideConnected)
{
    b2DistanceJointDef jointDef = b2DefaultDistanceJointDef();
    jointDef.bodyIdA = b1.body();
    jointDef.bodyIdB = b2.body();
    jointDef.localAnchorA = b2Vec2(anchorA.x(), anchorA.y());
    jointDef.localAnchorB = b2Vec2(anchorB.x(), anchorB.y());
    jointDef.collideConnected = collideConnected;
    jointDef.length = length;
    return b2CreateDistanceJoint(b2World, &jointDef);
}

}
