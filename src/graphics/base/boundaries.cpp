#include "graphics/base/boundaries.h"

#include "core/inf/variable.h"

#include "graphics/util/vec3_type.h"

namespace ark {

Boundaries::Boundaries()
    : Boundaries(V3(-0.5), V3(0.5))
{
}

Boundaries::Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax)
    : _aabb_min(std::move(aabbMin)), _aabb_max(std::move(aabbMax)), _size(Vec3Type::sub(_aabb_max, _aabb_min))
{
}

Boundaries::Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax, sp<Vec3> size)
    : _aabb_min(std::move(aabbMin)), _aabb_max(std::move(aabbMax)), _size(std::move(size))
{
}

Boundaries::Boundaries(const V3& aabbMin, const V3& aabbMax)
    : Boundaries(sp<Vec3::Const>::make(aabbMin), sp<Vec3::Const>::make(aabbMax), sp<Vec3::Const>::make(aabbMax - aabbMin))
{
}

Boundaries::Boundaries(const V3& position, const V3& size, const V3& origin)
    : Boundaries(position - origin, position - origin + size)
{
}

const sp<Vec3>& Boundaries::aabbMin() const
{
    return _aabb_min;
}

const sp<Vec3>& Boundaries::aabbMax() const
{
    return _aabb_max;
}

const sp<Vec3>& Boundaries::size() const
{
    return _size;
}

}
