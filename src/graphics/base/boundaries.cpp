#include "graphics/base/boundaries.h"

#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/util/vec3_type.h"

namespace ark {

Boundaries::Boundaries()
    : Boundaries(V3(-0.5f), V3(0.5f))
{
}

Boundaries::Boundaries(sp<Vec3> size)
    : _aabb_max(Vec3Type::mul(size, V3(0.5f))), _aabb_min(Vec3Type::negative(_aabb_max)), _size(std::move(size))
{
}

Boundaries::Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax)
    : _aabb_max(std::move(aabbMax)), _aabb_min(std::move(aabbMin)), _size(Vec3Type::sub(_aabb_max, _aabb_min))
{
}

Boundaries::Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax, sp<Vec3> size)
    : _aabb_max(std::move(aabbMax)), _aabb_min(std::move(aabbMin)), _size(std::move(size))
{
}

Boundaries::Boundaries(const V3& aabbMin, const V3& aabbMax)
    : Boundaries(sp<Vec3>::make<Vec3::Const>(aabbMin), sp<Vec3>::make<Vec3::Const>(aabbMax), sp<Vec3>::make<Vec3::Const>(aabbMax - aabbMin))
{
}

Boundaries::Boundaries(const V3& position, const V3& size, const V3& origin)
    : Boundaries(position - origin, position - origin + size)
{
}

const sp<Vec3>& Boundaries::size() const
{
    return _size;
}

const sp<Vec3>& Boundaries::aabbMin() const
{
    return _aabb_min;
}

const sp<Vec3>& Boundaries::aabbMax() const
{
    return _aabb_max;
}

sp<Boundaries> Boundaries::translate(sp<Vec3> xyz) const
{
    sp<Vec3> aabbMin = Vec3Type::add(_aabb_min, xyz);
    return sp<Boundaries>::make(std::move(aabbMin), Vec3Type::add(_aabb_max, std::move(xyz)), _size);
}

V2 Boundaries::toPivotPosition(const V2& size) const
{
    const V3& occupyAABBMin = _aabb_min->val();
    const V3& occupyAABBMax = _aabb_max->val();
    return {Math::lerp(0, size.x(), occupyAABBMin.x(), occupyAABBMax.x(), 0), Math::lerp(0, size.y(), occupyAABBMin.y(), occupyAABBMax.y(), 0)};
}

V3 Boundaries::toPivotPosition(const V3& size) const
{
    const V3& occupyAABBMin = _aabb_min->val();
    const V3& occupyAABBMax = _aabb_max->val();
    return {Math::lerp(0, size.x(), occupyAABBMin.x(), occupyAABBMax.x(), 0), Math::lerp(0, size.y(), occupyAABBMin.y(), occupyAABBMax.y(), 0), Math::lerp(0, size.z(), occupyAABBMin.z(), occupyAABBMax.z(), 0)};
}

bool Boundaries::update(uint64_t timestamp) const
{
    return UpdatableUtil::update(timestamp, _aabb_max, _aabb_min, _size);
}

}
