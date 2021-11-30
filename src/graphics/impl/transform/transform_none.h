#ifndef ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_NONE_H_
#define ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_NONE_H_

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

namespace ark {

class TransformNone : public Transform::Delegate {
public:

    virtual void snapshot(const Transform& transform, const V3& postTranslate, Transform::Snapshot& snapshot) const override;
    virtual V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
    virtual M4 toMatrix(const Transform::Snapshot& snapshot) const override;

private:
    struct Snapshot {
        V3 postTranslate;
    };

};

}

#endif
