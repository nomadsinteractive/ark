#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Boundaries {
public:
    Boundaries();
    Boundaries(sp<Vec3> position, sp<Vec3> extent);
//  [[script::bindings::auto]]
    Boundaries(sp<Vec3> aabbMin, sp<Vec3> aabbMax, sp<Vec3> center = nullptr, sp<Vec3> size = nullptr);
    Boundaries(const V3& aabbMin, const V3& aabbMax);

//  [[script::bindings::property]]
    const sp<Vec3>& size() const;
//  [[script::bindings::property]]
    const sp<Vec3>& center() const;

//  [[script::bindings::property]]
    const sp<Vec3>& aabbMin() const;
//  [[script::bindings::property]]
    const sp<Vec3>& aabbMax() const;

//  [[script::bindings::auto]]
    sp<Boundaries> translate(sp<Vec3> xyz) const;
//  [[script::bindings::auto]]
    sp<Boundaries> freeze() const;

//  [[script::bindings::auto]]
    V2 toPivotPosition(const V2& size) const;
//  [[script::bindings::auto]]
    V3 toPivotPosition(const V3& size) const;

//  [[script::bindings::auto]]
    sp<Boolean> ptin(sp<Vec3> point) const;
//  [[script::bindings::auto]]
    bool ptin(const V3& point) const;

    bool update(uint32_t timestamp) const;

private:
    sp<Vec3> _aabb_min;
    sp<Vec3> _aabb_max;
    sp<Vec3> _center;
    sp<Vec3> _size;
};

}

