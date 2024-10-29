#pragma once

#include "core/types/owned_ptr.h"

#include "bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

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
