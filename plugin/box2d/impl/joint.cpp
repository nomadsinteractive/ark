#include "box2d/impl/joint.h"

#include "core/ark.h"

#include "app/base/application_context.h"

#include "box2d/impl/rigid_body_box2d.h"
#include "box2d/impl/collider_box2d.h"

namespace ark {
namespace plugin {
namespace box2d {

Joint::Joint(const sp<ColliderBox2D>& world, b2Joint* joint)
    : _stub(sp<Stub>::make(world, joint))
{
    world->track(_stub);
}

Joint::~Joint()
{
    if(_stub->_world && _stub->_world->world().IsLocked())
        Ark::instance().applicationContext()->deferUnref(std::move(_stub));
}

void Joint::dispose()
{
    _stub->dispose();
}

void Joint::release()
{
    _stub->release();
}

Joint::Stub::Stub(const sp<ColliderBox2D>& world, b2Joint* joint)
    : _world(world), _joint(joint)
{
}

Joint::Stub::~Stub()
{
    if(_joint)
        dispose();
}

void Joint::Stub::dispose()
{
    DCHECK(!_world || _joint, "Joint has already been destroyed");
    if(_joint)
    {
        DCHECK(!_world->world().IsLocked(), "Cannot destroy joint in the middle of a time step");
        _world->world().DestroyJoint(_joint);
        _joint = nullptr;
    }
}

void Joint::Stub::release()
{
    _joint = nullptr;
    _world = nullptr;
}

}
}
}
