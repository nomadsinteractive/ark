#ifndef ARK_APP_UTIL_RIGID_BODY_DEF_H_
#define ARK_APP_UTIL_RIGID_BODY_DEF_H_

#include "core/types/box.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API RigidBodyDef {
public:
    RigidBodyDef() = default;
    RigidBodyDef(const V3& size, const V3& pivot, Box impl);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RigidBodyDef);

//  [[script::bindings::property]]
    const V3& size() const;
//  [[script::bindings::property]]
    const V3& pivot() const;

//  [[script::bindings::property]]
    float occupyRadius() const;

    const Box& impl() const;

private:
    float calcOccupyRadius(const V3& size, const V3& pivot) const;

private:
    V3 _size;
    V3 _pivot;
    float _occupy_radius;

    Box _impl;
};

}

#endif
