#pragma once

#include "renderer/base/animation_uploader.h"
#include "renderer/inf/animation.h"

namespace ark::plugin::gltf {

class AnimationGltf final : public Animation {
public:
    AnimationGltf(uint32_t durationInTicks, Table<String, uint32_t> node, std::vector<AnimationFrame> animationFrames);

    std::vector<std::pair<String, sp<Mat4>>> getNodeTranforms(sp<Numeric> tick) override;

private:
    uint32_t _duration_in_ticks;
    sp<Table<String, uint32_t>> _nodes;
    sp<std::vector<AnimationFrame>> _animation_frames;
};

}
