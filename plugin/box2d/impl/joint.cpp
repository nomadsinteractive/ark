#include "box2d/impl/joint.h"

#include "plugin/box2d/impl/body.h"
#include "plugin/box2d/impl/world.h"

namespace ark {
namespace plugin {
namespace box2d {

Joint::Joint(const sp<World>& world, b2Joint* joint)
    : _world(world), _joint(joint)
{
}

Joint::~Joint()
{
    if(_joint)
        _world->world().DestroyJoint(_joint);
}

b2Joint* Joint::object()
{
    return _joint;
}

void Joint::destroy()
{
    DCHECK(!_world || _joint, "Joint has already been destroyed");
    if(_joint)
    {
        _world->world().DestroyJoint(_joint);
        _joint = nullptr;
    }
}

void Joint::release()
{
    _joint = nullptr;
    _world = nullptr;
}

}
}
}
