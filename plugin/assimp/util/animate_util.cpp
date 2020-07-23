#include "assimp/util/animate_util.h"

#include "core/base/string.h"

namespace ark {
namespace plugin {
namespace assimp {

void AnimateUtil::loadNodeHierarchy(const aiAnimation* animation, float duration, const aiNode* node, const aiMatrix4x4& parentTransform, const NodeLoaderCallback& callback)
{
    const String nodeName(node->mName.data);
    const aiNodeAnim* pNodeAnim = findNodeAnim(animation, nodeName);

    aiMatrix4x4 nodeTransformation(node->mTransformation);

    if(pNodeAnim)
    {
        aiMatrix4x4 matScale = AnimateUtil::interpolateScale(duration, pNodeAnim);
        aiMatrix4x4 matRotation = AnimateUtil::interpolateRotation(duration, pNodeAnim);
        aiMatrix4x4 matTranslation = AnimateUtil::interpolateTranslation(duration, pNodeAnim);

        nodeTransformation = matTranslation * matRotation * matScale;
    }

    const aiMatrix4x4 globalTransformation = parentTransform * nodeTransformation;
    callback(nodeName, globalTransformation);

    for (uint32_t i = 0; i < node->mNumChildren; i++)
        loadNodeHierarchy(animation, duration, node->mChildren[i], globalTransformation, callback);
}

aiMatrix4x4 AnimateUtil::interpolateTranslation(float time, const aiNodeAnim* pNodeAnim)
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

aiMatrix4x4 AnimateUtil::interpolateRotation(float time, const aiNodeAnim* pNodeAnim)
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

aiMatrix4x4 AnimateUtil::interpolateScale(float time, const aiNodeAnim* pNodeAnim)
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

const aiNodeAnim* AnimateUtil::findNodeAnim(const aiAnimation* animation, const String& nodeName)
{
    for (uint32_t i = 0; i < animation->mNumChannels; ++i)
    {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];
        if (nodeName == nodeAnim->mNodeName.C_Str())
            return nodeAnim;
    }
    return nullptr;
}

}
}
}
