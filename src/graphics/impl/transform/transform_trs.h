#ifndef ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_TRS_H_
#define ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_TRS_H_

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"
#include "graphics/base/transform.h"

namespace ark {

class TransformTRS : public Transform::Delegate {
public:

    virtual void snapshot(const Transform& transform, const V3& postTranslate, Transform::Snapshot& snapshot) const override;
    virtual V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
    virtual M4 toMatrix(const Transform::Snapshot& snapshot) const override;

private:
    struct Snapshot {
        M3 matrix;
    };
};

}

#endif
