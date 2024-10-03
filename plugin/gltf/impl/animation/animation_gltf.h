#pragma once

#include "renderer/inf/animation.h"

namespace ark::plugin::gltf {

class AnimationGltf final : public Animation {
public:

    sp<AnimationUploader> makeInput(sp<Numeric> duration) override;
    const std::vector<String>& nodeNames() override;
};

}
