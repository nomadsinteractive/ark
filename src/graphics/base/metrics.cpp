#include "graphics/base/metrics.h"

namespace ark {

Metrics::Metrics()
    : Metrics(V3(-0.5), V3(0.5))
{
}

Metrics::Metrics(const V3& aabbMin, const V3& aabbMax)
    : _size(aabbMax - aabbMin), _aabb_min(aabbMin), _aabb_max(aabbMax)
{
}

Metrics::Metrics(const V3& position, const V3& size, const V3& origin)
    : Metrics(position - origin, position - origin + size)
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

Metrics Metrics::unit()
{
    return Metrics(V3(-0.5), V3(0.5));
}

}
