#include "graphics/base/transform_3d.h"

#include "graphics/util/transform_type.h"

namespace ark {

Transform3D::Transform3D(sp<Vec4> rotation, sp<Vec3> scale, sp<Vec3> translation)
    : TransformImpl(TransformType::TYPE_LINEAR_3D, std::move(rotation), std::move(scale), std::move(translation))
{
}

}
