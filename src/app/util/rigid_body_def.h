#pragma once

#include "core/types/box.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API [[deprecated]] RigidbodyDef {
public:
    RigidbodyDef() = default;
    RigidbodyDef(const V3& size, const V3& pivot, Box impl);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RigidbodyDef);

    const V3& size() const;
    const V3& pivot() const;

    float occupyRadius() const;

    const Box& impl() const;

private:
    V3 _size;
    V3 _pivot;
    float _occupy_radius;

    Box _impl;
};

}
