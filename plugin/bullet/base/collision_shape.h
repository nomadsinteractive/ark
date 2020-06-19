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
    CollisionShape(btCollisionShape* shape, btScalar mass);
    virtual ~CollisionShape() = default;

    btCollisionShape* btShape() const;

    btScalar mass() const;
    void setMass(btScalar mass);

private:
    op<btCollisionShape> _shape;
    btScalar _mass;
};

}
}
}

#endif
