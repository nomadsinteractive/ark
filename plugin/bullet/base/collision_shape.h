#ifndef ARK_PLUGIN_BULLET_BASE_SHAPE_H_
#define ARK_PLUGIN_BULLET_BASE_SHAPE_H_

#include "core/types/owned_ptr.h"
#include "core/types/shared_ptr.h"

#include "bullet/base/collider_bullet.h"

namespace ark {
namespace plugin {
namespace bullet {

class CollisionShape {
public:
    CollisionShape(ColliderBullet world, btCollisionShape* shape);

    btCollisionShape* btShape() const;

private:
    ColliderBullet _world;
    op<btCollisionShape> _shape;
};

}
}
}

#endif
