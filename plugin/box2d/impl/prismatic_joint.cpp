#include "box2d/impl/prismatic_joint.h"

#include "box2d/impl/rigidbody_box2d.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/collider_box2d.h"

namespace ark {
namespace plugin {
namespace box2d {

PrismaticJoint::PrismaticJoint(const sp<ColliderBox2D>& world, const RigidbodyBox2D& b1, const RigidbodyBox2D& b2, const V2& anchorA, const V2& anchorB, float referenceAngle, bool collideConnected)
    : Joint(world, makeJoint(world->world(), b1, b2, anchorA, anchorB, referenceAngle, collideConnected))
{
}

b2Joint*PrismaticJoint::makeJoint(b2World& b2World, const RigidbodyBox2D& b1, const RigidbodyBox2D& b2, const V2& anchorA, const V2& anchorB, float referenceAngle, bool collideConnected)
{
    b2PrismaticJointDef jointDef;
    jointDef.bodyA = b1.body();
    jointDef.bodyB = b2.body();
    jointDef.localAnchorA = b2Vec2(anchorA.x(), anchorA.y());
    jointDef.localAnchorB = b2Vec2(anchorB.x(), anchorB.y());
    jointDef.referenceAngle = referenceAngle;
    jointDef.collideConnected = collideConnected;
    return b2World.CreateJoint(&jointDef);

}

}
}
}
