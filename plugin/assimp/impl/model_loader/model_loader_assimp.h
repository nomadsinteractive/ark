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

class ModelImporterAssimp : public ModelBundle::Importer {
public:
    ModelImporterAssimp(Ark::RendererCoordinateSystem coordinateSystem);

    virtual Model import(const document& manifest, const Rect& uvBounds) override;

//  [[plugin::resource-loader::by-value("assimp")]]
    class BUILDER : public Builder<ModelBundle::Importer> {
    public:
        BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ModelBundle::Importer> build(const Scope& args) override;

    private:
        Ark::RendererCoordinateSystem _coordinate_system;
    };

private:
    Model loadModel(const aiScene* scene, const Rect& uvBounds, const sp<Assimp::Importer>& importer) const;
    Mesh loadMesh(const aiMesh* mesh, const Rect& uvBounds, element_index_t vertexBase, NodeTable& boneMapping) const;
    void loadBones(const aiMesh* mesh, NodeTable& boneMapping, Array<Mesh::BoneInfo>& bones) const;
    Table<String, sp<AnimateMaker>> loadAnimates(const aiScene* scene, const sp<Assimp::Importer>& importer, const NodeTable& nodes, const AnimateMakerAssimpNodes::NodeLoaderCallback& callback) const;

    bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
    array<element_index_t> loadIndices(const aiMesh* mesh, element_index_t indexOffset) const;

    void loadNodeHierarchy(const aiNode* node, NodeTable& nodes, std::unordered_map<uint32_t, uint32_t>& nodeIds) const;

    void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);


private:
    Ark::RendererCoordinateSystem _coordinate_system;

    std::vector<sp<Texture>> _textures;
};

}
}
}
#endif
