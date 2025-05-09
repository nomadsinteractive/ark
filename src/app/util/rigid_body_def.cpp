#include "app/util/rigid_body_def.h"

#include "core/util/math.h"

namespace ark {

RigidbodyDef::RigidbodyDef(const V3& size, const V3& pivot, Box impl)
    : _size(size), _pivot(pivot), _occupy_radius(calcOccupyRadius(size, pivot)), _impl(std::move(impl))
{
}

const V3& RigidbodyDef::size() const
{
    return _size;
}

const V3& RigidbodyDef::pivot() const
{
    return _pivot;
}

float RigidbodyDef::occupyRadius() const
{
    return _occupy_radius;
}

const Box& RigidbodyDef::impl() const
{
    return _impl;
}

float RigidbodyDef::calcOccupyRadius(const V3& size, const V3& pivot) const
{
    const V3 pm(std::max(std::abs(pivot.x()), std::abs(1.0f - pivot.x())), std::max(std::abs(pivot.y()), std::abs(1.0f - pivot.y())), std::max(std::abs(pivot.z()), std::abs(1.0f - pivot.z())));
    const V3 sm = pm * size;
    return Math::sqrt(Math::hypot2(sm));
}

}
