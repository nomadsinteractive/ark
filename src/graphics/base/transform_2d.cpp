#include "graphics/base/transform_2d.h"

namespace ark {

Transform2D::Transform2D(sp<Vec4> rotation, sp<Vec3> scale, sp<Vec3> translation)
    : Transform(std::move(rotation), std::move(scale), std::move(translation), TYPE_LINEAR_2D) {
}

}