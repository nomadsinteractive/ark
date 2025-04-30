#pragma once

#include "core/types/shared_ptr.h"

#include "bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

class CollisionShapeRef {
public:
    CollisionShapeRef(sp<btCollisionShape> shape, btScalar mass);
    virtual ~CollisionShapeRef() = default;

    btCollisionShape* btShape() const;

    btScalar mass() const;
    void setMass(btScalar mass);

protected:
    sp<btCollisionShape> _shape;
    btScalar _mass;
};

}
