#pragma once

#include "core/types/shared_ptr.h"

#include "bullet/base/collider_bullet.h"

namespace ark::plugin::bullet {

class CollisionShapeRef {
public:
    CollisionShapeRef(sp<btCollisionShape> shape, V3 size = V3(0));
    virtual ~CollisionShapeRef() = default;

    const sp<btCollisionShape>& btShape() const;

    V3 size() const;

protected:
    sp<btCollisionShape> _shape;
    V3 _size;
};

}
