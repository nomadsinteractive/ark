#pragma once

#include "core/types/optional.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

struct SearchingNode {

    bool operator < (const SearchingNode& other) const;
    bool operator == (const SearchingNode& other) const;

    int32_t _id;
    V3 _position;
    Optional<bool> _is_goal;
    Optional<float> _weight;
};

}
