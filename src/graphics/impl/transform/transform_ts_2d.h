#pragma once

#include "graphics/forwarding.h"
#include "graphics/impl/transform/transform_impl.h"

namespace ark {

class TransformTS2D final : public Transform {
public:
    TransformTS2D(const TransformImpl& transform);

    bool update(uint64_t timestamp) override;
    Snapshot snapshot() override;

    V4 transform(const Snapshot& snapshot, const V4& xyzw) override;
    M4 toMatrix(const Snapshot& snapshot) override;

private:
    sp<TransformImpl::Stub> _stub;
};

}
