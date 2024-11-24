#include "gltf/impl/animation/animation_gltf.h"

#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/mat.h"

namespace ark::plugin::gltf {

namespace {

struct AnimationSession {

    AnimationSession(sp<Numeric> tick, uint32_t durationInTicks, sp<std::vector<AnimationFrame>> animationFrames)
        : _tick(std::move(tick)), _duration_in_ticks(durationInTicks), _animation_frames(std::move(animationFrames)), _frame_index(static_cast<uint32_t>(fmod(_tick->val(), _duration_in_ticks))) {
    }

    bool update(uint64_t timestamp)
    {
        if(_tick->update(timestamp))
        {
            _frame_index = static_cast<uint32_t>(fmod(_tick->val(), _duration_in_ticks));
            return true;
        }
        return false;
    }

    sp<Numeric> _tick;
    uint32_t _duration_in_ticks;
    sp<std::vector<AnimationFrame>> _animation_frames;

    uint32_t _frame_index;
};

struct NodeMatrix final : Mat4 {
    NodeMatrix(sp<AnimationSession> session, uint32_t nodeIndex)
        : _session(std::move(session)), _node_index(nodeIndex) {
    }

    bool update(uint64_t timestamp) override
    {
        return _session->update(timestamp);
    }

    M4 val() override
    {
        return _session->_animation_frames->at(_session->_frame_index).at(_node_index);
    }

    sp<AnimationSession> _session;
    uint32_t _node_index;
};

}

AnimationGltf::AnimationGltf(uint32_t durationInTicks, Table<String, uint32_t> nodes, std::vector<AnimationFrame> animationFrames)
    : Animation(durationInTicks / 24.0f, 24.0f), _duration_in_ticks(durationInTicks), _nodes(sp<Table<String, uint32_t>>::make(std::move(nodes))), _animation_frames(sp<std::vector<AnimationFrame>>::make(std::move(animationFrames)))
{
}

std::vector<std::pair<String, sp<Mat4>>> AnimationGltf::getNodeTranforms(sp<Numeric> tick)
{
    std::vector<std::pair<String, sp<Mat4>>> nodeTransforms;

    sp<AnimationSession> session = sp<AnimationSession>::make(tick, _duration_in_ticks, _animation_frames);
    for(const auto& [name, nodeIdx] : *_nodes)
        nodeTransforms.emplace_back(name, sp<NodeMatrix>::make(session, nodeIdx));

    return nodeTransforms;
}

}
