#ifndef ARK_GRAPHICS_BASE_METRICS_H_
#define ARK_GRAPHICS_BASE_METRICS_H_

#include "core/base/api.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Metrics {
public:
    Metrics();
    Metrics(const V3& aabb, const V3& size, const V3& origin);
    Metrics(const V3& aabbMin, const V3& aabbMax, const V3& boundsMin, const V3& boundsMax);

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;
//  [[script::bindings::property]]
    float depth() const;

//  [[script::bindings::property]]
    const V3& aabb() const;
//  [[script::bindings::property]]
    const V3& aabbMin() const;
//  [[script::bindings::property]]
    const V3& aabbMax() const;
//  [[script::bindings::property]]
    const V3& size() const;
[[deprecated]]
//  [[script::bindings::property]]
    const V3& origin() const;

private:
    V3 _aabb;
    V3 _aabb_min;
    V3 _aabb_max;
    V3 _size;
    V3 _bounds_min;
    V3 _bounds_max;
    V3 _origin;
};

}
#endif
