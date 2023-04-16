#pragma once

#include <array>
#include <deque>

#include "core/forwarding.h"
#include "core/base/api.h"

namespace ark {

class ARK_API ConvexHullResolver {
public:
//  [[script::bindings::constructor]]
    ConvexHullResolver(bool checkError = true);

//  [[script::bindings::auto]]
    std::deque<std::array<int32_t, 2>> resolve(const std::deque<std::array<int32_t, 2>>& points);

//  [[script::bindings::auto]]
    std::deque<std::array<float, 2>> resolve(const std::deque<std::array<float, 2>>& points);

private:
    bool _check_error;
};

}
