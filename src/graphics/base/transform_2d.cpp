#include "graphics/base/transform_2d.h"

#include "graphics/util/transform_type.h"

namespace ark {

Transform2D::Transform2D(sp<Vec4> rotation, sp<Vec3> scale, sp<Vec3> translation)
    : TransformImpl(TransformType::TYPE_LINEAR_2D, std::move(rotation), std::move(scale), std::move(translation)) {
}

}
