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
//  [[script::bindings::property]]
    static uint32_t ticks(const sp<Animation>& self);
//  [[script::bindings::property]]
    static float tps(const sp<Animation>& self);
//  [[script::bindings::property]]
    static float duration(const sp<Animation>& self);

//  [[script::bindings::classmethod]]
    static std::vector<std::pair<String, sp<Mat4>>> getNodeTransforms(const sp<Animation>& self, sp<Integer> tick);
//  [[script::bindings::classmethod]]
    static std::vector<std::pair<String, sp<Mat4>>> getNodeTransforms(const sp<Animation>& self, sp<Numeric> tick);
};

}
