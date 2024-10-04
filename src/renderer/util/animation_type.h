#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

//[[script::bindings::class("Animation")]]
class ARK_API AnimationType {
public:
//  [[script::bindings::classmethod]]
    static sp<AnimationUploader> makeInput(const sp<Animation>& self, const sp<Numeric>& tick, const sp<Runnable>& onComplete = nullptr);

//  [[script::bindings::property]]
    static float ticks(const sp<Animation>& self);
//  [[script::bindings::property]]
    static float tps(const sp<Animation>& self);
//  [[script::bindings::property]]
    static float duration(const sp<Animation>& self);
//  [[script::bindings::property]]
    static const std::vector<String>& nodeNames(const sp<Animation>& self);

//  [[script::bindings::classmethod]]
    static std::vector<std::pair<String, sp<Mat4>>> getNodeTransforms(const sp<Animation>& self, sp<Numeric> time);
};

}
