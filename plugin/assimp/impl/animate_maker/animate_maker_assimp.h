#ifndef ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATE_MAKER_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATE_MAKER_ASSIMP_H_

#include <unordered_map>

#include <assimp/Importer.hpp>

#include <assimp/anim.h>
#include <assimp/scene.h>

#include "core/base/string.h"
#include "core/inf/array.h"
#include "core/inf/flatable.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/mat.h"

#include "renderer/inf/animate_maker.h"

namespace ark {
namespace plugin {
namespace assimp {

class AnimateMakerAssimp : public AnimateMaker {
public:
    AnimateMakerAssimp(sp<Assimp::Importer> importer, const aiAnimation* animation, const aiNode* rootNode, const std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping);

    virtual sp<Animate> makeAnimate(const sp<Numeric>& duration) override;

private:
    class BoneInfo : public Mat4 {
    public:

        virtual bool update(uint64_t timestamp) override;
        virtual M4 val() override;

        aiMatrix4x4 _offset;
        aiMatrix4x4 _intermediate_transform;

        M4 _final_transform;
    };

    class AnimateImpl : public Animate {
    public:
        AnimateImpl(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, const std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping);

        virtual bool update(uint64_t timestamp) override;

        virtual void flat(void* buf) override;
        virtual uint32_t size() override;

    private:
        void updateHierarchy(float time);
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

    };

private:
    sp<Assimp::Importer> _importer;
    const aiAnimation* _animation;
    const aiNode* _root_node;

    std::unordered_map<String, std::pair<size_t, aiMatrix4x4>> _bone_mapping;
};

}
}
}

#endif
