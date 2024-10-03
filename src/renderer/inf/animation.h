#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API Animation {
public:
    Animation(float duration, float tps)
        : _duration(duration), _tps(tps) {
    }

    virtual ~Animation() = default;

    virtual sp<AnimationUploader> makeInput(sp<Numeric> duration) = 0;
    virtual const std::vector<String>& nodeNames() = 0;

    float duration() const {
        return _duration;
    }

    float tps() const {
        return _tps;
    }

protected:
    float _duration;
    float _tps;
};

}
