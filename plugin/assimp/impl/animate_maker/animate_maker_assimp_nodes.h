#ifndef ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATE_MAKER_ASSIMP_NODES_H_
#define ARK_PLUGIN_ASSIMP_IMPL_ANIMATE_MAKER_ANIMATE_MAKER_ASSIMP_NODES_H_

#include <unordered_map>

#include <assimp/Importer.hpp>

#include <assimp/anim.h>
#include <assimp/scene.h>

#include "core/base/string.h"
#include "core/collection/table.h"
#include "core/inf/flatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/mat.h"

#include "renderer/inf/animate_maker.h"

#include "assimp/base/node_table.h"
#include "assimp/util/animate_util.h"

namespace ark {
namespace plugin {
namespace assimp {

class AnimateMakerAssimpNodes : public AnimateMaker {
public:
    typedef std::function<void(Table<String, Node>& nodes, const String& nodeName, const aiMatrix4x4& globalTransformation)> NodeLoaderCallback;

    AnimateMakerAssimpNodes(sp<Assimp::Importer> importer, const aiAnimation* animation, const aiNode* rootNode, NodeTable nodes, NodeLoaderCallback callback);

    virtual sp<Animate> makeAnimate(const sp<Numeric>& duration) override;

private:
    class AnimateImpl : public Animate {
    public:
        AnimateImpl(const sp<Numeric>& duration, const aiAnimation* animation, const aiNode* node, const Table<String, Node>& nodes, NodeLoaderCallback callback);

        virtual bool update(uint64_t timestamp) override;

        virtual void flat(void* buf) override;
        virtual uint32_t size() override;

    private:
        void updateHierarchy(float time);
        void loadNodeHierarchy(float duration, const aiNode* node, const aiMatrix4x4& parentTransform, const NodeLoaderCallback& callback);


    private:
        float _ticks_per_sec;
        float _duration_in_ticks;
        aiMatrix4x4 _global_inversed_transform;

        sp<Numeric> _duration;

        const aiAnimation* _animation;
        const aiNode* _root_node;

        Table<String, Node> _nodes;
        NodeLoaderCallback _callback;
    };

private:
    sp<Assimp::Importer> _importer;
    const aiAnimation* _animation;
    const aiNode* _root_node;

    NodeTable _nodes;
    NodeLoaderCallback _callback;
};

}
}
}

#endif
