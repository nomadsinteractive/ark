#include "box2d/impl/prismatic_joint.h"

#include "box2d/impl/body.h"
#include "box2d/impl/joint.h"
#include "box2d/impl/world.h"

namespace ark {
namespace plugin {
namespace box2d {

PrismaticJoint::PrismaticJoint(const sp<World>& world, const Body& b1, const Body& b2, const V2& anchorA, const V2& anchorB, float referenceAngle, bool collideConnected)
{
    b2PrismaticJointDef jointDef;
    jointDef.bodyA = b1.body();
    jointDef.bodyB = b2.body();
    jointDef.localAnchorA = b2Vec2(anchorA.x(), anchorA.y());
    jointDef.localAnchorB = b2Vec2(anchorB.x(), anchorB.y());
    jointDef.referenceAngle = referenceAngle;
    jointDef.collideConnected = collideConnected;
    b2Joint* joint = world->world().CreateJoint(&jointDef);
    _joint = sp<Joint>::make(world, joint);
    world->track(_joint);
}

}
}
}
