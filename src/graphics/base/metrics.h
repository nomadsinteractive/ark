#pragma once

#include "core/base/api.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Metrics {
public:
    Metrics();
    Metrics(const V3& aabbMin, const V3& aabbMax);
    Metrics(const V3& position, const V3& size, const V3& origin);

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;
//  [[script::bindings::property]]
    float depth() const;

//  [[script::bindings::property]]
    const V3& aabbMin() const;
//  [[script::bindings::property]]
    const V3& aabbMax() const;
//  [[script::bindings::property]]
    const V3& size() const;

    static Metrics unit();

private:
    V3 _size;
    V3 _aabb_min;
    V3 _aabb_max;
};

}

