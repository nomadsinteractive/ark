#include "graphics/base/boundaries.h"

#include "core/inf/variable.h"
#include "core/util/math.h"
#include "core/util/updatable_util.h"

#include "graphics/util/vec3_type.h"

namespace ark {

namespace {

bool ptinAABB(const V3 point, const V3 aabbMin, const V3 aabbMax)
{
    for(int32_t i = 0; i < 3; i++)
        if(!Math::between(aabbMin[i], aabbMax[i], point[i]))
            return false;
    return true;
}

class InBoundaries final : public Boolean {
public:
    InBoundaries(sp<Vec3> point, sp<Vec3> aabbMin, sp<Vec3> aabbMax)
        : _point(std::move(point)), _aabb_min(std::move(aabbMin)), _aabb_max(std::move(aabbMax))
    {
    }

    bool update(uint32_t tick) override
    {
        return UpdatableUtil::update(tick, _point, _aabb_min, _aabb_max);
    }

    bool val() override
    {
        return ptinAABB(_point->val(), _aabb_min->val(), _aabb_max->val());
    }

private:
    sp<Vec3> _point;
    sp<Vec3> _aabb_min;
    sp<Vec3> _aabb_max;
};

}

Boundaries::Boundaries()
    : Boundaries(V3(-0.5f), V3(0.5f))
{
}

Boundaries::Boundaries(sp<Vec3> position, sp<Vec3> extent)
    : _aabb_min(Vec3Type::sub(position, extent)), _aabb_max(Vec3Type::add(position, extent)), _center(std::move(position)), _size(Vec3Type::mul(std::move(extent), 2.0f))
{
}

Boundaries::Boundaries(const V3& aabbMin, const V3& aabbMax)
    : Boundaries(sp<Vec3>::make<Vec3::Const>(aabbMin), sp<Vec3>::make<Vec3::Const>(aabbMax), sp<Vec3>::make<Vec3::Const>(aabbMax - aabbMin))
{
}

Boundaries::Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax, sp<Vec3> size)
    : _aabb_min(std::move(aabbMin)), _aabb_max(std::move(aabbMax)), _center(Vec3Type::mul(Vec3Type::add(_aabb_min, _aabb_max), 0.5f)),
      _size(size ? std::move(size) : Vec3Type::sub(_aabb_max, _aabb_min))
{
}

const sp<Vec3>& Boundaries::size() const
{
    return _size;
}

const sp<Vec3>& Boundaries::center() const
{
    return _center;
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

bool Boundaries::ptin(const V3& point) const
{
    return ptinAABB(point, _aabb_min->val(), _aabb_max->val());;
}

sp<Boolean> Boundaries::ptin(sp<Vec3> point) const
{
    return sp<Boolean>::make<InBoundaries>(std::move(point), _aabb_min, _aabb_max);
}

bool Boundaries::update(const uint32_t timestamp) const
{
    return UpdatableUtil::update(timestamp, _aabb_max, _aabb_min, _size);
}

}
