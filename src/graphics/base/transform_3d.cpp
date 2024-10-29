#include "graphics/base/transform_3d.h"

namespace ark {

Transform3D::Transform3D(sp<Vec4> rotation, sp<Vec3> scale, sp<Vec3> translation)
    : Transform(std::move(rotation), std::move(scale), std::move(translation), TYPE_LINEAR_3D)
{
}

}
