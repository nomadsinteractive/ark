#pragma once

#include "renderer/base/animation_uploader.h"
#include "renderer/inf/animation.h"

namespace ark::plugin::gltf {

class AnimationGltf final : public Animation {
public:
    AnimationGltf(uint32_t durationInTicks, std::vector<String> nodeNames, Table<String, uint32_t> node, std::vector<AnimationFrame> animationFrames);

    sp<AnimationUploader> makeInput(sp<Numeric> duration) override;
    const std::vector<String>& nodeNames() override;

private:
    uint32_t _duration_in_ticks;
    std::vector<String> _node_names;

    sp<Table<String, uint32_t>> _nodes;
    sp<std::vector<AnimationFrame>> _animation_frames;
};

}
