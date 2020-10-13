#include "assimp/impl/animation/animation_assimp_nodes.h"

#include "core/inf/variable.h"

#include "graphics/base/mat.h"

namespace ark {
namespace plugin {
namespace assimp {

AnimationAssimpNodes::AnimationAssimpNodes(float tps, const aiAnimation* animation, const aiNode* rootNode, const aiMatrix4x4& globalTransform, Table<String, Node>& nodes, const NodeLoaderCallback& callback)
    : _ticks_per_sec(tps), _nodes(sp<Table<String, uint32_t>>::make()), _matrics(sp<std::vector<M4>>::make())
{
    aiMatrix4x4 globalInversedTransform = rootNode->mTransformation * globalTransform;
    globalInversedTransform.Inverse();

    float ticksPerSec = animation->mTicksPerSecond != 0 ? static_cast<float>(animation->mTicksPerSecond) : 25.0f;
    float durationInTicks = static_cast<float>(animation->mDuration);

    _duration_in_ticks = static_cast<uint32_t>(durationInTicks * _ticks_per_sec / ticksPerSec);
    float step = durationInTicks / static_cast<float>(_duration_in_ticks);
    for(uint32_t i = 0; i < _duration_in_ticks; ++i)
        loadHierarchy(static_cast<float>(i) * step, rootNode, animation, globalInversedTransform, nodes, callback, _matrics);

     _duration = durationInTicks / ticksPerSec;

     uint32_t index = 0;
     for(const auto& iter : nodes)
         _nodes->push_back(iter.first, index++);
}

sp<AnimationInput> AnimationAssimpNodes::makeTransforms(const sp<Numeric>& duration)
{
    return sp<AnimationInput>::make<AnimationInputImpl>(duration, _ticks_per_sec, _duration_in_ticks, _nodes, _matrics);
}

float AnimationAssimpNodes::duration()
{
    return _duration;
}

const std::vector<String>& AnimationAssimpNodes::nodeNames()
{
    return _nodes->keys();
}

AnimationAssimpNodes::AnimationInputImpl::AnimationInputImpl(const sp<Numeric>& duration, float ticksPerSecond, float durationInTicks, const sp<Table<String, uint32_t> >& node, const sp<std::vector<M4>>& matrics)
    : _stub(sp<Stub>::make(duration, ticksPerSecond, durationInTicks, node, matrics))
{
}

sp<Mat4> AnimationAssimpNodes::AnimationInputImpl::getNodeMatrix(const String& name)
{
    return sp<NodeMatrix>::make(_stub, name);
}

bool AnimationAssimpNodes::AnimationInputImpl::update(uint64_t timestamp)
{
    return _stub->update(timestamp);
}

void AnimationAssimpNodes::AnimationInputImpl::flat(void* buf)
{
    _stub->flat(buf);
}

uint32_t AnimationAssimpNodes::AnimationInputImpl::size()
{
    return _stub->_node_size * sizeof(M4);
}

AnimationAssimpNodes::AnimationInputImpl::Stub::Stub(const sp<Numeric>& duration, float ticksPerSecond, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& nodes, const sp<std::vector<M4>>& matrics)
    : _ticks_per_sec(ticksPerSecond), _duration_in_ticks(durationInTicks), _duration(duration), _nodes(nodes), _matrics(matrics), _node_size(_nodes->size()), _tick(0)
{
}

bool AnimationAssimpNodes::AnimationInputImpl::Stub::update(uint64_t timestamp)
{
    if(_duration->update(timestamp))
    {
        _tick = static_cast<uint32_t>(fmod(_duration->val() * _ticks_per_sec, _duration_in_ticks));
        return true;
    }
    return false;
}

void AnimationAssimpNodes::AnimationInputImpl::Stub::flat(void* buf)
{
    M4* mat = reinterpret_cast<M4*>(buf);
    const M4* frameInput = getFrameInput();
    for(size_t i = 0; i < _node_size; ++i)
        mat[i] = frameInput[i];
}

const M4* AnimationAssimpNodes::AnimationInputImpl::Stub::getFrameInput() const
{
    return &_matrics->at(_tick * _node_size);
}

void AnimationAssimpNodes::loadHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& transform, Table<String, Node>& nodes, const NodeLoaderCallback& callback, std::vector<M4>& output)
{
    loadNodeHierarchy(tick, node, animation, aiMatrix4x4(), nodes, callback);
    for(Node& i : nodes.values())
    {
        const aiMatrix4x4 finalMatrix = transform * i._intermediate;
        output.push_back(M4(finalMatrix).transpose());
    }
}

void AnimationAssimpNodes::loadNodeHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& parentTransform, Table<String, Node>& nodes, const NodeLoaderCallback& callback)
{
    const String nodeName(node->mName.data);
    const aiNodeAnim* pNodeAnim = AnimateUtil::findNodeAnim(animation, nodeName);

    aiMatrix4x4 nodeTransformation(node->mTransformation);

    if(pNodeAnim)
    {
        aiMatrix4x4 matScale = AnimateUtil::interpolateScale(tick, pNodeAnim);
        aiMatrix4x4 matRotation = AnimateUtil::interpolateRotation(tick, pNodeAnim);
        aiMatrix4x4 matTranslation = AnimateUtil::interpolateTranslation(tick, pNodeAnim);

        nodeTransformation = matTranslation * matRotation * matScale;
    }

    const aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;

    auto iter = nodes.find(nodeName);
    if (iter != nodes.end())
        iter->second._intermediate = callback(iter->second, globalTransformation);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
        loadNodeHierarchy(tick, node->mChildren[i], animation, globalTransformation, nodes, callback);
}

AnimationAssimpNodes::AnimationInputImpl::NodeMatrix::NodeMatrix(const sp<Stub>& stub, const String& name)
    : _stub(stub), _node_index(_stub->_nodes->at(name))
{
}

bool AnimationAssimpNodes::AnimationInputImpl::NodeMatrix::update(uint64_t timestamp)
{
    return _stub->update(timestamp);
}

M4 AnimationAssimpNodes::AnimationInputImpl::NodeMatrix::val()
{
    return _stub->getFrameInput()[_node_index];
}

}
}
}
