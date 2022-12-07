#include "graphics/base/metrics.h"

namespace ark {

Metrics::Metrics()
    : Metrics(V3(1.0), V3(1.0), V3(0))
{
}

Metrics::Metrics(const V3& aabb, const V3& size, const V3& origin)
    : _aabb(aabb), _size(size), _origin(origin)
{
}

Metrics::Metrics(const V3& aabbMin, const V3& aabbMax, const V3& boundsMin, const V3& boundsMax)
    : _aabb(aabbMax - aabbMin), _aabb_min(aabbMin), _aabb_max(aabbMax), _size(boundsMax - boundsMin), _bounds_min(boundsMin), _bounds_max(boundsMax), _origin(-aabbMin)
{
}

float Metrics::width() const
{
    return _size.x();
}

float Metrics::height() const
{
    return _size.y();
}

float Metrics::depth() const
{
    return _size.z();
}

const V3& Metrics::aabb() const
{
    return _aabb;
}

const V3& Metrics::aabbMin() const
{
    return _aabb_min;
}

const V3& Metrics::aabbMax() const
{
    return _aabb_max;
}

const V3& Metrics::size() const
{
    return _size;
}

const V3& Metrics::origin() const
{
    return _origin;
}

}
