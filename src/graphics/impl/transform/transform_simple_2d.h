#ifndef ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_SIMPLE_2D_H_
#define ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_SIMPLE_2D_H_

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

namespace ark {

class TransformSimple2D : public Transform::Delegate {
public:

    virtual void snapshot(const Transform& transform, Transform::Snapshot& snapshot) const override;
    virtual V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
    virtual M4 toMatrix(const Transform::Snapshot& snapshot) const override;

private:
    struct Snapshot {
        uint32_t magic;
        V2 scale;
        V2 pivot;
    };
};

}

#endif
