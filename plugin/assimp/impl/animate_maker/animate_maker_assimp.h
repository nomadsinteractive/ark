#ifndef ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATE_MAKER_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATE_MAKER_ASSIMP_H_

#include <vector>

#include <assimp/anim.h>
#include <assimp/scene.h>

#include "core/inf/array.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "renderer/inf/animate_maker.h"

namespace ark {
namespace plugin {
namespace assimp {

class AnimateMakerAssimp : public AnimateMaker {
public:
    AnimateMakerAssimp(const aiAnimation& animation, const aiNode& rootNode, const std::unordered_map<String, size_t>& boneMapping);

    virtual array<sp<Mat4>> makeAnimate(const sp<Numeric>& duration) override;

    class BoneInfo;

    class Animate : public Array<sp<Mat4>> {
    public:
        Animate(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, const std::unordered_map<String, size_t>& boneMapping);

        virtual size_t length() override;

        virtual sp<Mat4>* buf() override;

    private:
        void update(float time);
        void readNodeHierarchy(float duration, const aiNode* node, const aiMatrix4x4& parentTransform);

    private:
        float _ticks_per_sec;
        float _duration_in_ticks;
        aiMatrix4x4 _global_inversed_transform;

        sp<Numeric> _duration;

        const aiAnimation* _animation;
        const aiNode* _root_node;

        std::vector<sp<BoneInfo>> _bone_infos;
        std::vector<sp<Mat4>> _matrices;

        std::unordered_map<String, size_t> _bone_mapping;

        float _last_updated;
    };

    class BoneInfo : public Mat4 {
    public:

        virtual bool update(uint64_t timestamp) override;
        virtual M4 val() override;

        aiMatrix4x4 _offset;
        aiMatrix4x4 _intermediate_transform;
        aiMatrix4x4 _final_transform;

    };

private:
    const aiAnimation* _animation;
    const aiNode* _root_node;

    std::unordered_map<String, size_t> _bone_mapping;
};

}
}
}

#endif
