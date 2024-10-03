#include "renderer/base/animation_uploader.h"

#include "core/collection/table.h"
#include "core/base/string.h"
#include "core/inf/writable.h"

#include "graphics/base/mat.h"

namespace ark {

AnimationUploader::AnimationUploader(sp<Numeric> duration, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& node, const sp<std::vector<AnimationFrame>>& animationFrames)
    : Uploader(node->size() * sizeof(M4)), _stub(sp<Stub>::make(std::move(duration), durationInTicks, node, animationFrames))
{
}

sp<Mat4> AnimationUploader::getNodeMatrix(const String& name)
{
    return sp<NodeMatrix>::make(_stub, name);
}

std::vector<float> AnimationUploader::getTransformVariance(const V3& c, const std::vector<String>& nodes)
{
    Table<String, uint32_t>& nodeTable = _stub->_nodes;
    const std::vector<AnimationFrame>& animationFrames = _stub->_animation_frames;
    std::vector<float> avgs(nodes.size()), lastFrame(nodes.size());
    for(size_t i = 0; i < animationFrames.size(); ++i)
    {
        const AnimationFrame& f1 = animationFrames.at(i);
        const V4 scala = V4(c, 1.0f);

        for(size_t j = 0; j < nodes.size(); ++j)
        {
            const String& name = nodes.at(j);
            uint32_t nodeIndex = nodeTable.at(name);
            const V4 d = f1[nodeIndex] * scala - scala;
            float sl = d.dot(d);
            if(i != 0)
                avgs[j] = sl - lastFrame[j];
            lastFrame[j] = sl;
        }
    }
    return avgs;
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

AnimationUploader::NodeMatrix::NodeMatrix(const sp<Stub>& stub, const String& name)
    : _stub(stub), _node_index(_stub->_nodes->at(name))
{
}

bool AnimationUploader::NodeMatrix::update(uint64_t timestamp)
{
    return _stub->update(timestamp);
}

M4 AnimationUploader::NodeMatrix::val()
{
    return _stub->getFrameInput()[_node_index];
}
}
