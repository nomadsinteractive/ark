#include "box2d/impl/joint.h"

#include "core/ark.h"

#include "app/base/application_context.h"

#include "box2d/impl/rigidbody_box2d.h"
#include "box2d/impl/collider_box2d.h"

namespace ark {
namespace plugin {
namespace box2d {

Joint::Joint(const sp<ColliderBox2D>& world, b2JointId joint)
    : _stub(sp<Stub>::make(world, joint))
{
    world->track(_stub);
}

Joint::~Joint()
{
}

void Joint::dispose()
{
    _stub->dispose();
}

void Joint::release()
{
    _stub->release();
}

Joint::Stub::Stub(const sp<ColliderBox2D>& world, b2JointId joint)
    : _world(world), _joint(joint)
{
}

Joint::Stub::~Stub()
{
    if(b2Joint_IsValid(_joint))
        dispose();
}

void Joint::Stub::dispose()
{
    DCHECK(!_world || b2Joint_IsValid(_joint), "Joint has already been destroyed");
    if(b2Joint_IsValid(_joint))
    {
        b2DestroyJoint(_joint);
        _joint = b2_nullJointId;
    }
}

void Joint::Stub::release()
{
    _joint = b2_nullJointId;
    _world = nullptr;
}

}
}
}
