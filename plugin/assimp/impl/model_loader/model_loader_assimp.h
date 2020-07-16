#ifndef ARK_PLUGIN_ASSIMP_IMPL_MODEL_LOADER_MODEL_LOADER_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_MODEL_LOADER_MODEL_LOADER_ASSIMP_H_

#include <assimp/postprocess.h>
#include <assimp/version.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

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

namespace ark {
namespace plugin {
namespace assimp {

class ModelLoaderAssimp : public ModelLoader {
public:
    ModelLoaderAssimp(sp<ModelBundle> atlas);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;
    virtual Model loadModel(int32_t type) override;

//  [[plugin::builder("assimp")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<Builder<ModelBundle>> _model_bundle;
    };

//  [[plugin::builder::by-value("assimp")]]
    class IMPORTER_BUILDER : public Builder<ModelBundle::Importer> {
    public:
        IMPORTER_BUILDER() = default;

        virtual sp<ModelBundle::Importer> build(const Scope& args) override;

    };

private:
    bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
    void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);

    class Importer : public ModelBundle::Importer {
    public:
        Importer();

        virtual Model import(const String& src, const Rect& uvBounds) override;

    private:
        Mesh loadMesh(const aiMesh* mesh, const Rect& uvBounds, element_index_t vertexBase, std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping) const;
        Model loadModel(const aiScene* scene, const Rect& uvBounds) const;
        void loadBones(const aiMesh* mesh, std::unordered_map<String, std::pair<size_t, aiMatrix4x4>>& boneMapping, Array<Mesh::BoneInfo>& bones) const;

        bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
        array<element_index_t> loadIndices(const aiMesh* mesh, element_index_t indexOffset) const;

    private:
        Assimp::Importer _importer;

    };

private:
    Importer _importer;

    sp<ModelBundle> _model_bundle;
    std::vector<sp<Texture>> _textures;
};

}
}
}
#endif
