#include "bullet/base/collision_shape.h"

namespace ark {
namespace plugin {
namespace bullet {

CollisionShape::CollisionShape(World world, btCollisionShape* shape)
    : _world(std::move(world)), _shape(shape)
{
}

btCollisionShape* CollisionShape::btShape() const
{
    return _shape.get();
}

}
}
}
