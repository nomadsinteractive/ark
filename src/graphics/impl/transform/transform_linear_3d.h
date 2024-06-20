#pragma once

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/base/transform.h"

namespace ark {

class TransformLinear3D final : public Transform::Delegate {
public:

    bool update(const Transform::Stub& transform, uint64_t timestamp) override;
    void snapshot(const Transform::Stub& transform, Transform::Snapshot& snapshot) const override;
    V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
    M4 toMatrix(const Transform::Snapshot& snapshot) const override;

};

}
