#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

namespace ark {

class ARK_API Boundaries {
public:
    Boundaries();
    Boundaries(sp<Vec3> size);
    Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax);
    Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax, sp<Vec3> size);
    Boundaries(const V3& aabbMin, const V3& aabbMax);
    Boundaries(const V3& position, const V3& size, const V3& origin);

//  [[script::bindings::property]]
    const sp<Vec3>& size() const;

//  [[script::bindings::property]]
    const sp<Vec3>& aabbMin() const;
//  [[script::bindings::property]]
    const sp<Vec3>& aabbMax() const;

//  [[script::bindings::auto]]
    sp<Boundaries> translate(sp<Vec3> xyz) const;

    bool update(uint64_t timestamp) const;

private:
    sp<Vec3> _aabb_max;
    sp<Vec3> _aabb_min;
    sp<Vec3> _size;
};

}

