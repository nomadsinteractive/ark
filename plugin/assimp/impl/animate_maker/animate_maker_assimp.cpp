#include "assimp/impl/animate_maker/animate_maker_assimp.h"

#include "graphics/base/mat.h"

namespace ark {
namespace plugin {
namespace assimp {

static aiMatrix4x4 interpolateTranslation(float time, const aiNodeAnim* pNodeAnim)
{
    aiVector3D translation;

    if (pNodeAnim->mNumPositionKeys == 1)
    {
        translation = pNodeAnim->mPositionKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
        {
            if (time < static_cast<float>(pNodeAnim->mPositionKeys[i + 1].mTime))
            {
                frameIndex = i;
                break;
            }
        }

        aiVectorKey currentFrame = pNodeAnim->mPositionKeys[frameIndex];
        aiVectorKey nextFrame = pNodeAnim->mPositionKeys[(frameIndex + 1) % pNodeAnim->mNumPositionKeys];

        float delta = (time - static_cast<float>(currentFrame.mTime)) / static_cast<float>(nextFrame.mTime - currentFrame.mTime);

        const aiVector3D& start = currentFrame.mValue;
        const aiVector3D& end = nextFrame.mValue;

        translation = (start + delta * (end - start));
    }

    aiMatrix4x4 mat;
    aiMatrix4x4::Translation(translation, mat);
    return mat;
}

// Returns a 4x4 matrix with interpolated rotation between current and next frame
static aiMatrix4x4 interpolateRotation(float time, const aiNodeAnim* pNodeAnim)
{
    aiQuaternion rotation;

    if (pNodeAnim->mNumRotationKeys == 1)
    {
        rotation = pNodeAnim->mRotationKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
        {
            if (time < static_cast<float>(pNodeAnim->mRotationKeys[i + 1].mTime))
            {
                frameIndex = i;
                break;
            }
        }

        aiQuatKey currentFrame = pNodeAnim->mRotationKeys[frameIndex];
        aiQuatKey nextFrame = pNodeAnim->mRotationKeys[(frameIndex + 1) % pNodeAnim->mNumRotationKeys];

        float delta = (time - static_cast<float>(currentFrame.mTime)) / static_cast<float>(nextFrame.mTime - currentFrame.mTime);

        const aiQuaternion& start = currentFrame.mValue;
        const aiQuaternion& end = nextFrame.mValue;

        aiQuaternion::Interpolate(rotation, start, end, delta);
        rotation.Normalize();
    }

    aiMatrix4x4 mat(rotation.GetMatrix());
    return mat;
}


// Returns a 4x4 matrix with interpolated scaling between current and next frame
static aiMatrix4x4 interpolateScale(float time, const aiNodeAnim* pNodeAnim)
{
    aiVector3D scale;

    if (pNodeAnim->mNumScalingKeys == 1)
    {
        scale = pNodeAnim->mScalingKeys[0].mValue;
    }
    else
    {
        uint32_t frameIndex = 0;
        for (uint32_t i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
        {
            if (time < static_cast<float>(pNodeAnim->mScalingKeys[i + 1].mTime))
            {
                frameIndex = i;
                break;
            }
        }

        aiVectorKey currentFrame = pNodeAnim->mScalingKeys[frameIndex];
        aiVectorKey nextFrame = pNodeAnim->mScalingKeys[(frameIndex + 1) % pNodeAnim->mNumScalingKeys];

        float delta = (time - static_cast<float>(currentFrame.mTime)) / static_cast<float>(nextFrame.mTime - currentFrame.mTime);

        const aiVector3D& start = currentFrame.mValue;
        const aiVector3D& end = nextFrame.mValue;

        scale = (start + delta * (end - start));
    }

    aiMatrix4x4 mat;
    aiMatrix4x4::Scaling(scale, mat);
    return mat;
}

static const aiNodeAnim* findNodeAnim(const aiAnimation& animation, const String& nodeName)
{
    for (uint32_t i = 0; i < animation.mNumChannels; ++i)
    {
        const aiNodeAnim* nodeAnim = animation.mChannels[i];
        if (nodeName == nodeAnim->mNodeName.C_Str())
            return nodeAnim;
    }
    return nullptr;
}

AnimateMakerAssimp::AnimateMakerAssimp(sp<Assimp::Importer> importer, const aiAnimation* animation, const aiNode* rootNode, const std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping)
    : _importer(std::move(importer)), _animation(animation), _root_node(rootNode), _bone_mapping(boneMapping)
{
}

sp<Animate> AnimateMakerAssimp::makeAnimate(const sp<Numeric>& duration)
{
    return sp<AnimateImpl>::make(duration, _animation, _root_node, _bone_mapping);
}

AnimateMakerAssimp::AnimateImpl::AnimateImpl(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, const std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping)
    : _ticks_per_sec(static_cast<float>(animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f)), _duration_in_ticks(static_cast<float>(animation->mDuration)),
      _global_inversed_transform(node->mTransformation), _duration(duration), _animation(animation), _root_node(node), _bone_infos(boneMapping.size()), _matrices(boneMapping.size())
{
    _global_inversed_transform.Inverse();

    for(size_t i = 0; i < _bone_infos.size(); ++i)
    {
        sp<BoneInfo> boneInfo = sp<BoneInfo>::make();
        _matrices[i] = boneInfo;
        _bone_infos[i] = std::move(boneInfo);
    }

    for(const auto& i : boneMapping)
    {
        size_t index = i.second.first;
        _bone_mapping.insert(std::make_pair(i.first, index));
        _bone_infos.at(index)->_offset = i.second.second;
    }

    updateHierarchy(_duration->val());
}

bool AnimateMakerAssimp::AnimateImpl::update(uint64_t timestamp)
{
    if(_duration->update(timestamp))
    {
        updateHierarchy(_duration->val());
        return true;
    }
    return false;
}

void AnimateMakerAssimp::AnimateImpl::flat(void* buf)
{
    M4* mat = reinterpret_cast<M4*>(buf);
    for(size_t i = 0; i < _matrices.size(); ++i)
        mat[i] = _matrices.at(i)->val();
}

uint32_t AnimateMakerAssimp::AnimateImpl::size()
{
    return _matrices.size() * sizeof(M4);
}

void AnimateMakerAssimp::AnimateImpl::updateHierarchy(float time)
{
    float animationTime = fmod(time * _ticks_per_sec, _duration_in_ticks);

    aiMatrix4x4 identity = aiMatrix4x4();
    readNodeHierarchy(animationTime, _root_node, identity);

    for(const sp<BoneInfo>& i : _bone_infos)
    {
        const aiMatrix4x4 finalMatrix = _global_inversed_transform * i->_intermediate_transform;
        i->_final_transform = M4(finalMatrix).transpose();
    }
}

void AnimateMakerAssimp::AnimateImpl::readNodeHierarchy(float duration, const aiNode* node, const aiMatrix4x4& parentTransform)
{
    const String nodeName(node->mName.data);
    const aiNodeAnim* pNodeAnim = findNodeAnim(*_animation, nodeName);

    aiMatrix4x4 nodeTransformation(node->mTransformation);

    if(pNodeAnim)
    {
        aiMatrix4x4 matScale = interpolateScale(duration, pNodeAnim);
        aiMatrix4x4 matRotation = interpolateRotation(duration, pNodeAnim);
        aiMatrix4x4 matTranslation = interpolateTranslation(duration, pNodeAnim);

        nodeTransformation = matTranslation * matRotation * matScale;
    }

    const aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;
    const auto iter = _bone_mapping.find(nodeName);
    if (iter != _bone_mapping.end())
    {
        uint32_t boneIndex = iter->second;
        const sp<AnimateMakerAssimp::BoneInfo>& boneInfo = _bone_infos.at(boneIndex);
        boneInfo->_intermediate_transform = globalTransformation * boneInfo->_offset;
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
        readNodeHierarchy(duration, node->mChildren[i], globalTransformation);
}

bool AnimateMakerAssimp::BoneInfo::update(uint64_t /*timestamp*/)
{
    return true;
}

M4 AnimateMakerAssimp::BoneInfo::val()
{
    return _final_transform;
}

}
}
}
