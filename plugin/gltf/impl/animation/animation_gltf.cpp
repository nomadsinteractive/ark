#include "gltf/impl/animation/animation_gltf.h"

#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/mat.h"

namespace ark::plugin::gltf {

namespace {

struct AnimationSession {

    AnimationSession(sp<Integer> tick, uint32_t durationInTicks, sp<std::vector<AnimationFrame>> animationFrames)
        : _tick(std::move(tick)), _duration_in_ticks(durationInTicks), _animation_frames(std::move(animationFrames)) {
    }

    bool update(uint64_t timestamp) const
    {
        return _tick->update(timestamp);
    }

    const AnimationFrame& currentFrame() const
    {
        return _animation_frames->at(static_cast<uint32_t>(_tick->val()) % _duration_in_ticks);
    }

private:
    sp<Integer> _tick;
    uint32_t _duration_in_ticks;
    sp<std::vector<AnimationFrame>> _animation_frames;
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
        return _session->currentFrame().at(_node_index);
    }

    sp<AnimationSession> _session;
    uint32_t _node_index;
};

}

AnimationGltf::AnimationGltf(uint32_t durationInTicks, Table<String, uint32_t> nodes, std::vector<AnimationFrame> animationFrames)
    : Animation(durationInTicks / 24.0f, 24.0f), _duration_in_ticks(durationInTicks), _nodes(sp<Table<String, uint32_t>>::make(std::move(nodes))), _animation_frames(sp<std::vector<AnimationFrame>>::make(std::move(animationFrames)))
{
}

std::vector<std::pair<String, sp<Mat4>>> AnimationGltf::getNodeTranforms(sp<Integer> tick)
{
    std::vector<std::pair<String, sp<Mat4>>> nodeTransforms;

    sp<AnimationSession> session = sp<AnimationSession>::make(std::move(tick), _duration_in_ticks, _animation_frames);
    for(const auto& [name, nodeIdx] : *_nodes)
        nodeTransforms.emplace_back(name, sp<NodeMatrix>::make(session, nodeIdx));

    return nodeTransforms;
}

}
