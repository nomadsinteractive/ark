#ifndef ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATION_ASSIMP_NODES_H_
#define ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATION_ASSIMP_NODES_H_

#include <unordered_map>

#include <assimp/Importer.hpp>

#include <assimp/anim.h>
#include <assimp/scene.h>

#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/mat.h"

#include "renderer/inf/animation.h"
#include "renderer/inf/animation_input.h"

#include "assimp/base/node_table.h"
#include "assimp/util/animate_util.h"

namespace ark {
namespace plugin {
namespace assimp {

class AnimationAssimpNodes : public Animation {
public:
    typedef std::function<aiMatrix4x4(const Node& node, const aiMatrix4x4& globalTransformation)> NodeLoaderCallback;

    AnimationAssimpNodes(float tps, const aiAnimation* animation, const aiNode* rootNode, const aiMatrix4x4& globalTransform, Table<String, Node>& nodes, const NodeLoaderCallback& callback);

    virtual sp<AnimationInput> makeTransforms(sp<Numeric> duration) override;
    virtual float duration() override;
    virtual const std::vector<String>& nodeNames() override;

private:
    class AnimationInputImpl : public AnimationInput {
    public:
        AnimationInputImpl(sp<Numeric> duration, float ticksPerSecond, float durationInTicks, const sp<Table<String, uint32_t>>& node, const sp<std::vector<M4>>& matrics);

        virtual sp<Mat4> getNodeMatrix(const String& name) override;

        virtual bool update(uint64_t timestamp) override;

        virtual void flat(void* buf) override;
        virtual uint32_t size() override;

    private:
        struct Stub {
            Stub(sp<Numeric> duration, float ticksPerSecond, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& nodes, const sp<std::vector<M4>>& matrics);

            bool update(uint64_t timestamp);
            void flat(void* buf);

            const M4* getFrameInput() const;

            float _ticks_per_sec;
            uint32_t _duration_in_ticks;

            sp<Numeric> _duration;

            sp<Table<String, uint32_t>> _nodes;
            sp<std::vector<M4>> _matrics;
            size_t _node_size;

            uint32_t _tick;
        };

        class NodeMatrix : public Mat4 {
        public:
            NodeMatrix(const sp<Stub>& stub, const String& name);

            virtual bool update(uint64_t timestamp) override;

            virtual M4 val() override;

        private:
            sp<Stub> _stub;
            uint32_t _node_index;
        };

    private:
        sp<Stub> _stub;
    };

private:
    static void loadHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& parentTransform, Table<String, Node>& nodes, const NodeLoaderCallback& callback, std::vector<M4>& output);
    static void loadNodeHierarchy(float tick, const aiNode* node, const aiAnimation* animation, const aiMatrix4x4& parentTransform, Table<String, Node>& nodes, const NodeLoaderCallback& callback);

private:
    float _ticks_per_sec;
    uint32_t _duration_in_ticks;

    sp<Table<String, uint32_t>> _nodes;
    sp<std::vector<M4>> _matrics;

    float _duration;
};

}
}
}

#endif
