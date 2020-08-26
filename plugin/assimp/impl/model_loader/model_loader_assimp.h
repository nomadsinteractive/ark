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
#include "assimp/impl/animate_maker/animate_maker_assimp_nodes.h"

namespace ark {
namespace plugin {
namespace assimp {

class ModelImporterAssimp : public ModelLoader::Importer {
public:
    ModelImporterAssimp(Ark::RendererCoordinateSystem coordinateSystem, sp<MaterialBundle> materialBundle);

    virtual Model import(const document& manifest, const Rect& uvBounds) override;

    class BUILDER_IMPL : public Builder<ModelLoader::Importer> {
    public:
        BUILDER_IMPL(const sp<ResourceLoaderContext>& resourceLoaderContext, SafePtr<Builder<MaterialBundle>> materialBundle);

        virtual sp<ModelLoader::Importer> build(const Scope& args) override;

    private:
        Ark::RendererCoordinateSystem _coordinate_system;
        SafePtr<Builder<MaterialBundle>> _material_bundle;
    };

//  [[plugin::resource-loader::by-value("assimp")]]
    class VALUE_BUILDER : public Builder<ModelLoader::Importer> {
    public:
        VALUE_BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ModelLoader::Importer> build(const Scope& args) override;

    private:
        BUILDER_IMPL _impl;
    };

//  [[plugin::resource-loader("assimp")]]
    class BUILDER : public Builder<ModelLoader::Importer> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ModelLoader::Importer> build(const Scope& args) override;

    private:
        BUILDER_IMPL _impl;
    };

private:
    const aiScene* loadScene(const sp<Assimp::Importer>& importer, const String& src, bool checkMeshes = true) const;
    Model loadModel(const aiScene* scene, const Rect& uvBounds, const sp<Assimp::Importer>& importer, const document& manifest) const;
    Mesh loadMesh(const aiScene* scene, const aiMesh* mesh, const Rect& uvBounds, element_index_t vertexBase, NodeTable& boneMapping) const;
    NodeTable loadNodes(const aiNode* node, Model& model) const;
    void loadBones(const aiMesh* mesh, NodeTable& boneMapping, Array<Mesh::BoneInfo>& bones) const;
    void loadAnimates(Table<String, sp<AnimateMaker>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, const sp<Assimp::Importer>& importer, const NodeTable& nodes, const AnimateMakerAssimpNodes::NodeLoaderCallback& callback) const;
    void loadAnimates(Table<String, sp<AnimateMaker>>& animates, const aiScene* scene, const aiMatrix4x4& globalTransformation, sp<Assimp::Importer> importer, NodeTable nodes, AnimateMakerAssimpNodes::NodeLoaderCallback callback, String name, String alias) const;

    bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
    array<element_index_t> loadIndices(const aiMesh* mesh, element_index_t indexOffset) const;

    void loadNodeHierarchy(const aiNode* node, NodeTable& nodes, std::unordered_map<uint32_t, uint32_t>& nodeIds) const;

    void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);

    static void callbackNodeAnimation(Table<String, Node>& nodes, const String& nodeName, const aiMatrix4x4& transform);
    static void callbackBoneAnimation(Table<String, Node>& nodes, const String& nodeName, const aiMatrix4x4& transform);
private:
    Ark::RendererCoordinateSystem _coordinate_system;
    sp<MaterialBundle> _material_bundle;

    std::vector<sp<Texture>> _textures;
};

}
}
}
#endif
