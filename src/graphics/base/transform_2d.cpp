#include "graphics/base/transform_2d.h"

#include "core/types/global.h"

#include "graphics/base/quaternion.h"
#include "graphics/util/vec2_type.h"

namespace ark {

Transform2D::Transform2D(sp<Numeric> rotation, sp<Vec2> scale, sp<Vec2> translation)
    : TransformImpl(TransformType::TYPE_LINEAR_2D, rotation ? sp<Vec4>::make<Quaternion>(std::move(rotation), sp<Vec3>::make<Vec3::Const>(constants::AXIS_Z)) : nullptr,
                    scale ? Vec2Type::extend(std::move(scale), Global<Constants>()->NUMERIC_ONE) : nullptr,
                    translation ? Vec2Type::extend(std::move(translation), Global<Constants>()->NUMERIC_ZERO) : nullptr)
{
}

}
