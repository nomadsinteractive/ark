#ifndef ARK_PLUGIN_BULLET_BASE_SHAPE_H_
#define ARK_PLUGIN_BULLET_BASE_SHAPE_H_

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "bullet/base/world.h"

namespace ark {
namespace plugin {
namespace bullet {

class CollisionShape {
public:
    CollisionShape(World world, btCollisionShape* shape);

    btCollisionShape* btShape() const;

private:
    World _world;
    op<btCollisionShape> _shape;
};

}
}
}

#endif
