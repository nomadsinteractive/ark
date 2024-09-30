#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Animation {
public:
    Animation(float ticks, float tps)
        : _ticks(ticks), _tps(tps) {
    }

    virtual ~Animation() = default;

    virtual sp<AnimationInput> makeInput(sp<Numeric> duration) = 0;
    virtual const std::vector<String>& nodeNames() = 0;

    const float ticks() const {
        return _ticks;
    }

    const float tps() const {
        return _tps;
    }

protected:
    float _ticks;
    float _tps;
};

}
