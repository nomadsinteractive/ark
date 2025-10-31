#pragma once

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/impl/transform/transform_impl.h"

namespace ark {

class TransformTRS2D final : public Transform {
public:
    TransformTRS2D(const Transform& transform);

    bool update(uint32_t tick) override;
    Snapshot snapshot() override;
    V4 transform(const Snapshot& snapshot, const V4& xyzw) override;
    M4 toMatrix(const Snapshot& snapshot) override;
};

}
