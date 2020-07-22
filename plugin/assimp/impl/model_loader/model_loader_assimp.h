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

namespace ark {
namespace plugin {
namespace assimp {

class ModelLoaderAssimp {
public:
    ModelLoaderAssimp(sp<ModelBundle> atlas);

//  [[plugin::resource-loader::by-value("assimp")]]
    class IMPORTER_BUILDER : public Builder<ModelBundle::Importer> {
    public:
        IMPORTER_BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ModelBundle::Importer> build(const Scope& args) override;

    private:
        Ark::RendererCoordinateSystem _coordinate_system;
    };

private:
    bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
    void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);

    class Importer : public ModelBundle::Importer {
    public:
        Importer(Ark::RendererCoordinateSystem coordinateSystem);

        virtual Model import(const String& src, const Rect& uvBounds) override;

    private:
        Model loadModel(const aiScene* scene, const Rect& uvBounds, const sp<Assimp::Importer>& importer) const;
        Mesh loadMesh(const aiMesh* mesh, const Rect& uvBounds, element_index_t vertexBase, NodeMap& boneMapping) const;
        void loadBones(const aiMesh* mesh, NodeMap& boneMapping, Array<Mesh::BoneInfo>& bones) const;

        bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
        array<element_index_t> loadIndices(const aiMesh* mesh, element_index_t indexOffset) const;

        void loadNodeHierarchy(const aiNode* node, NodeMap& nodes, std::unordered_map<uint32_t, uint32_t>& nodeIds) const;

    private:
        Ark::RendererCoordinateSystem _coordinate_system;

    };

private:
    std::vector<sp<Texture>> _textures;
};

}
}
}
#endif
