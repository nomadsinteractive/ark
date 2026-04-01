#pragma once

#include "graphics/base/v3.h"

#include "renderer/components/varyings.h"

namespace ark {

struct LayerContextSnapshot {
    V3 _position;
    bool _dirty;
    bool _visible;
    bool _discarded;
    Varyings::Snapshot _varyings;
};

}
