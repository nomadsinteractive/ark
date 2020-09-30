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
    typedef std::function<void(Table<String, Node>& nodes, const String& nodeName, const aiMatrix4x4& globalTransformation)> NodeLoaderCallback;

    AnimationAssimpNodes(sp<Assimp::Importer> importer, const aiAnimation* animation, const aiNode* rootNode, const aiMatrix4x4& globalTransform, NodeTable nodes, NodeLoaderCallback callback);

    virtual sp<AnimationInput> makeTransforms(const sp<Numeric>& duration) override;
    virtual float duration() override;
    virtual const std::vector<String>& nodeNames() override;

private:
    class AnimationInputImpl : public AnimationInput {
    public:
        AnimationInputImpl(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, float ticksPerSecond, const aiMatrix4x4& globalTransform, const Table<String, Node>& nodes, NodeLoaderCallback callback);

        virtual sp<Mat4> getNodeMatrix(const String& name) override;

        virtual bool update(uint64_t timestamp) override;

        virtual void flat(void* buf) override;
        virtual uint32_t size() override;

    private:
        struct Stub {
            Stub(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, float ticksPerSecond, const aiMatrix4x4& globalTransform, const Table<String, Node>& nodes, NodeLoaderCallback callback);

            bool update(uint64_t timestamp);

            void updateHierarchy(float time);
            void loadNodeHierarchy(float duration, const aiNode* node, const aiMatrix4x4& parentTransform, const NodeLoaderCallback& callback);

            float _ticks_per_sec;
            float _duration_in_ticks;
            aiMatrix4x4 _global_inversed_transform;

            sp<Numeric> _duration;

            const aiAnimation* _animation;
            const aiNode* _root_node;

            Table<String, Node> _nodes;
            NodeLoaderCallback _callback;
        };

        class NodeMatrix : public Mat4 {
        public:
            NodeMatrix(const sp<Stub>& stub, const String& name);

            virtual bool update(uint64_t timestamp) override;

            virtual M4 val() override;

        private:
            sp<Stub> _stub;
            const Node& _node;
        };

    private:
        sp<Stub> _stub;
    };

private:
    sp<Assimp::Importer> _importer;
    const aiAnimation* _animation;
    const aiNode* _root_node;
    float _ticks_per_sec;

    aiMatrix4x4 _global_transform;

    NodeTable _nodes;
    NodeLoaderCallback _callback;

};

}
}
}

#endif
