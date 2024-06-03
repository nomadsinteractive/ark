#pragma once

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v4.h"

namespace ark {

struct Constants {
    Constants();

    sp<Boolean> BOOLEAN_TRUE;
    sp<Boolean> BOOLEAN_FALSE;
};

constexpr std::initializer_list<float> AXIS_X = {1.0f, 0, 0};
constexpr std::initializer_list<float> AXIS_Y = {0, 1.0f, 0};
constexpr std::initializer_list<float> AXIS_Z = {0, 0, 1.0f};

constexpr std::initializer_list<float> QUATERNION_ZERO = {0, 0, 0, 1.0f};

}
