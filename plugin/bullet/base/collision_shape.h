#pragma once

#include "core/types/shared_ptr.h"

#include "bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

class CollisionShape {
public:
    CollisionShape(sp<btCollisionShape> shape, btScalar mass);
    virtual ~CollisionShape() = default;

    const sp<btCollisionShape>& btShape() const;

    btScalar mass() const;
    void setMass(btScalar mass);

private:
    sp<btCollisionShape> _shape;
    btScalar _mass;
};

}
