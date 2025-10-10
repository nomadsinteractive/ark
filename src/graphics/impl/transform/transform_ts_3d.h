#pragma once

#include "graphics/forwarding.h"
#include "graphics/impl/transform/transform_impl.h"

namespace ark {

class TransformTS3D final : public Transform {
public:
    TransformTS3D(const Transform& transform);

    bool update(uint64_t timestamp) override;
    Snapshot snapshot() override;
    V4 transform(const Snapshot& snapshot, const V4& xyzw) override;
    M4 toMatrix(const Snapshot& snapshot) override;
};

}
