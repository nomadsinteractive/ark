#ifndef ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_LINEAR_2D_H_
#define ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_LINEAR_2D_H_

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/base/transform.h"

namespace ark {

class TransformLinear2D : public Transform::Delegate {
public:

    virtual void snapshot(const Transform& transform, Transform::Snapshot& snapshot) const override;
    virtual V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
    virtual M4 toMatrix(const Transform::Snapshot& snapshot) const override;

private:
    struct Snapshot {
        uint32_t magic;
        M3 matrix;
    };
};

}

#endif
