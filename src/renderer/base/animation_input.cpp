#include "renderer/base/animation_input.h"

#include "core/collection/table.h"
#include "core/base/string.h"

#include "graphics/base/mat.h"

namespace ark {

AnimationInput::AnimationInput(sp<Numeric> duration, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& node, const sp<std::vector<AnimationFrame>>& animationFrames)
    : _stub(sp<Stub>::make(std::move(duration), durationInTicks, node, animationFrames))
{
}

sp<Mat4> AnimationInput::getNodeMatrix(const String& name)
{
    return sp<NodeMatrix>::make(_stub, name);
}

std::vector<float> AnimationInput::getTransformVariance(const V3& c, const std::vector<String>& nodes)
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

bool AnimationInput::update(uint64_t timestamp)
{
    return _stub->update(timestamp);
}

void AnimationInput::flat(void* buf)
{
    _stub->flat(buf);
}

uint32_t AnimationInput::size()
{
    return static_cast<uint32_t>(_stub->_size);
}

AnimationInput::Stub::Stub(sp<Numeric> tick, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& nodes, const sp<std::vector<AnimationFrame>>& matrics)
    : _duration_in_ticks(durationInTicks), _tick(std::move(tick)), _nodes(nodes), _animation_frames(matrics), _size(_nodes->size() * sizeof(M4)), _frame_index(0)
{
}

bool AnimationInput::Stub::update(uint64_t timestamp)
{
    if(_tick->update(timestamp))
    {
        _frame_index = static_cast<uint32_t>(fmod(_tick->val(), _duration_in_ticks));
        return true;
    }
    return false;
}

void AnimationInput::Stub::flat(void* buf)
{
    const M4* frameInput = getFrameInput();
    memcpy(buf, frameInput, _size);
}

const M4* AnimationInput::Stub::getFrameInput() const
{
    return _animation_frames->at(_frame_index).data();
}


AnimationInput::NodeMatrix::NodeMatrix(const sp<Stub>& stub, const String& name)
    : _stub(stub), _node_index(_stub->_nodes->at(name))
{
}

bool AnimationInput::NodeMatrix::update(uint64_t timestamp)
{
    return _stub->update(timestamp);
}

M4 AnimationInput::NodeMatrix::val()
{
    return _stub->getFrameInput()[_node_index];
}
}
