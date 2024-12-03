#pragma once

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

typedef std::vector<M4> AnimationFrame;

class ARK_API Animation {
public:
    Animation(uint32_t durationInTicks, Table<String, uint32_t> nodes, std::vector<AnimationFrame> animationFrames);

//  [[script::bindings::property]]
    float duration() const;
//  [[script::bindings::property]]
    float tps() const;
//  [[script::bindings::property]]
    uint32_t ticks() const;

//  [[script::bindings::auto]]
    std::vector<std::pair<String, sp<Mat4>>> getLocalTransforms(sp<Integer> tick) const;
//  [[script::bindings::auto]]
    std::vector<std::pair<String, sp<Mat4>>> getLocalTransforms(sp<Numeric> tick) const;

//  [[script::bindings::auto]]
    sp<Mat4> getGlobalTransform(const Node& node, sp<Integer> tick) const;

private:
    float _tps;
    float _duration;
    uint32_t _duration_in_ticks;

    sp<Table<String, uint32_t>> _nodes;
    sp<std::vector<AnimationFrame>> _animation_frames;
};

}
