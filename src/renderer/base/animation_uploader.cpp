#include "renderer/base/animation_uploader.h"

#include "core/collection/table.h"
#include "core/base/string.h"
#include "core/inf/writable.h"

#include "graphics/base/mat.h"

namespace ark {

namespace {

struct NodeMatrix final : Mat4 {
    NodeMatrix(const sp<AnimationUploader::Stub>& stub, const String& name)
        : _stub(stub), _node_index(_stub->_nodes->at(name))
    {
    }

    bool update(uint64_t timestamp) override
    {
        return _stub->update(timestamp);
    }

    M4 val() override
    {
        return _stub->getFrameInput()[_node_index];
    }

    sp<AnimationUploader::Stub> _stub;
    uint32_t _node_index;
};

}

AnimationUploader::AnimationUploader(sp<Numeric> duration, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& nodeIds, const sp<std::vector<AnimationFrame>>& animationFrames)
    : Uploader(nodeIds->size() * sizeof(M4)), _stub(sp<Stub>::make(std::move(duration), durationInTicks, nodeIds, animationFrames))
{
}

sp<Mat4> AnimationUploader::getNodeMatrix(const String& name)
{
    return sp<NodeMatrix>::make(_stub, name);
}

std::vector<V3> AnimationUploader::getNodeDisplacements(const String& nodeName, const V3& origin) const
{
    const Table<String, uint32_t>& nodeTable = _stub->_nodes;
    const std::vector<AnimationFrame>& animationFrames = _stub->_animation_frames;
    const uint32_t nodeIndex = nodeTable.at(nodeName);
    const V4 scala(origin, 1.0f);
    std::vector<V3> displacements(animationFrames.size());

    for(size_t i = 0; i < animationFrames.size(); ++i)
        displacements[i] = (animationFrames[i][nodeIndex] * scala).toNonHomogeneous();

    return displacements;
}

bool AnimationUploader::update(uint64_t timestamp)
{
    return _stub->update(timestamp);
}

void AnimationUploader::upload(Writable& buf)
{
    buf.write(_stub->getFrameInput(), _size, 0);
}

AnimationUploader::Stub::Stub(sp<Numeric> tick, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& nodes, const sp<std::vector<AnimationFrame>>& matrics)
    : _duration_in_ticks(durationInTicks), _tick(std::move(tick)), _nodes(nodes), _animation_frames(matrics), _frame_index(0)
{
}

bool AnimationUploader::Stub::update(uint64_t timestamp)
{
    if(_tick->update(timestamp))
    {
        _frame_index = static_cast<uint32_t>(fmod(_tick->val(), _duration_in_ticks));
        return true;
    }
    return false;
}

const M4* AnimationUploader::Stub::getFrameInput() const
{
    DCHECK(_frame_index < _animation_frames->size(), "Animation has no frame[%d]", _frame_index);
    return _animation_frames->at(_frame_index).data();
}

}
