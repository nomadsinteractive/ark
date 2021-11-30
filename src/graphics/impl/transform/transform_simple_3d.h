#ifndef ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_SIMPLE_3D_H_
#define ARK_GRAPHICS_IMPL_TRANSFORM_TRANSFORM_SIMPLE_3D_H_

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

namespace ark {

class TransformSimple3D : public Transform::Delegate {
public:

    virtual void snapshot(const Transform& transform, const V3& postTranslate, Transform::Snapshot& snapshot) const override;
    virtual V3 transform(const Transform::Snapshot& snapshot, const V3& position) const override;
    virtual M4 toMatrix(const Transform::Snapshot& snapshot) const override;

private:
    struct Snapshot {
        uint32_t magic;
        V3 scale;
        V3 preTranslate;
        V3 postTranslate;
    };
};

}

#endif
