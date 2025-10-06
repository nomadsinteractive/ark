#include "bullet/base/collision_shape_ref.h"

namespace ark::plugin::bullet {

CollisionShapeRef::CollisionShapeRef(sp<btCollisionShape> shape, const V3 contentSize)
    : _shape(std::move(shape)), _size(contentSize)
{
}

const sp<btCollisionShape>& CollisionShapeRef::btShape() const
{
    return _shape;
}

V3 CollisionShapeRef::size() const
{
    return _size;
}

}
