#pragma once

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
#include "renderer/base/mesh.h"
#include "renderer/base/model_bundle.h"
#include "renderer/inf/model_loader.h"

#include "assimp/forwarding.h"

namespace ark::plugin::assimp {

class ModelImporterAssimp final : public ModelLoader::Importer {
public:
    typedef std::function<aiMatrix4x4(const AnimationNode& node, const aiMatrix4x4& globalTransformation)> NodeLoaderCallback;

    Model import(const Manifest& manifest, MaterialBundle& materialBundle) override;

//  [[plugin::builder::by-value("assimp")]]
    class BUILDER : public Builder<ModelLoader::Importer> {
    public:
        BUILDER() = default;

        sp<ModelLoader::Importer> build(const Scope& args) override;
    };

private:
    const aiScene* loadScene(Assimp::Importer& importer, const String& src, bool checkMeshes = true) const;
    Model loadModel(const aiScene* scene, MaterialBundle& materialBundle, const Manifest& manifest) const;
    Mesh loadMesh(const aiScene* scene, const aiMesh* mesh, MaterialBundle& materialBundle, uint32_t meshId, element_index_t vertexBase, NodeTable& boneMapping, const Vector<sp<Material> >& materials) const;
    NodeTable loadNodes(const aiNode* node, Model& model) const;
    void loadBones(const aiMesh* mesh, NodeTable& boneMapping, Array<Mesh::BoneInfo>& bones) const;
    void loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback) const;
    void loadAnimates(float tps, Table<String, sp<Animation>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, Table<String, AnimationNode>& nodes, const NodeLoaderCallback& callback, String name, String alias) const;

    bitmap loadBitmap(const sp<BitmapLoaderBundle>& imageResource, const aiTexture* tex) const;
    Vector<element_index_t> loadIndices(const aiMesh* mesh, element_index_t indexOffset) const;

    sp<Node> loadNodeHierarchy(WeakPtr<Node> parentNode, const aiNode* node, const Vector<sp<Mesh>>& meshes) const;

    void loadNodeHierarchy(const aiNode* node, NodeTable& nodes, std::unordered_map<uint32_t, uint32_t>& nodeIds) const;
    void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);
    Vector<sp<Material>> loadMaterials(const aiScene* scene, MaterialBundle& materialBundle) const;

private:
    Vector<sp<Texture>> _textures;
};

}
