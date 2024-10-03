#include "gltf/impl/animation/animation_gltf.h"

#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/mat.h"

namespace ark::plugin::gltf {

AnimationGltf::AnimationGltf(uint32_t durationInTicks, std::vector<String> nodeNames, Table<String, uint32_t> nodes, std::vector<AnimationFrame> animationFrames)
    : Animation(durationInTicks / 24.0f, 24.0f), _node_names(std::move(nodeNames)), _duration_in_ticks(durationInTicks), _nodes(sp<Table<String, uint32_t> >::make(std::move(nodes))), _animation_frames(sp<std::vector<AnimationFrame>>::make(std::move(animationFrames)))
{
}

sp<AnimationUploader> AnimationGltf::makeInput(sp<Numeric> duration)
{
    return sp<AnimationUploader>::make(std::move(duration), _duration_in_ticks, _nodes, _animation_frames);
}

const std::vector<String>& AnimationGltf::nodeNames()
{
    return _node_names;
}

}
