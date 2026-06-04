#pragma once

#include "core/types/optional.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

struct SearchingNode {
    V3i _id;
    V3 _position;
    Optional<bool> _is_goal;
    Optional<float> _weight;
};

}
