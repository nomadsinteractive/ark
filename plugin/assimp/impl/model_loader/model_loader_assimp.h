#ifndef ARK_PLUGIN_ASSIMP_IMPL_MODEL_LOADER_MODEL_LOADER_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_MODEL_LOADER_MODEL_LOADER_ASSIMP_H_

#include <assimp/postprocess.h>
#include <assimp/version.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

#include "core/ark.h"
#include "core/forwarding.h"
#include "core/inf/array.h"
#include "core/inf/builder.h"

#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/mesh.h"
#include "renderer/base/model_bundle.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/uploader.h"

#include "assimp/forwarding.h"
#include "assimp/impl/animation/animation_assimp_nodes.h"

namespace ark {
namespace plugin {
namespace assimp {

class ModelImporterAssimp : public ModelLoader::Importer {
public:

    virtual Model import(const document& manifest, MaterialBundle& materialBundle) override;

//  [[plugin::builder::by-value("assimp")]]
    class BUILDER : public Builder<ModelLoader::Importer> {
    public:
        BUILDER() = default;

        virtual sp<ModelLoader::Importer> build(const Scope& args) override;

    };

private:
    const aiScene* loadScene(Assimp::Importer& importer, const String& src, bool checkMeshes = true) const;
    Model loadModel(const aiScene* scene, MaterialBundle& materialBundle, const document& manifest) const;
    Mesh loadMesh(const aiScene* scene, const aiMesh* mesh, MaterialBundle& materialBundle, element_index_t vertexBase, NodeTable& boneMapping, const std::vector<sp<Material> >& materials) const;
    NodeTable loadNodes(const aiNode* node, Model& model) const;
    void loadBones(const aiMesh* mesh, NodeTable& boneMapping, Array<Mesh::BoneInfo>& bones) const;
    void loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, Node>& nodes, const AnimationAssimpNodes::NodeLoaderCallback& callback) const;
    void loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, Node>& nodes, const AnimationAssimpNodes::NodeLoaderCallback& callback, String name, String alias) const;

    bitmap loadBitmap(const sp<BitmapLoaderBundle>& imageResource, const aiTexture* tex) const;
    array<element_index_t> loadIndices(const aiMesh* mesh, element_index_t indexOffset) const;

    void loadNodeHierarchy(const aiNode* node, NodeTable& nodes, std::unordered_map<uint32_t, uint32_t>& nodeIds) const;

    void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);

    static void yUp2zUp(const Mesh& mesh, bool upSign);
    static V3 yUp2zUp(const V3& p, bool upSign);

    static aiMatrix4x4 callbackNodeAnimation(const Node& node, const aiMatrix4x4& transform);
    static aiMatrix4x4 callbackBoneAnimation(const Node& node, const aiMatrix4x4& transform);

private:
    std::vector<sp<Texture>> _textures;

};

}
}
}
#endif
