#ifndef ARK_PLUGIN_ASSIMP_IMPL_MODEL_LOADER_MODEL_LOADER_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_MODEL_LOADER_MODEL_LOADER_ASSIMP_H_

#include <unordered_map>

#include <assimp/postprocess.h>
#include <assimp/version.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

#include "core/forwarding.h"
#include "core/inf/builder.h"

#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/model_bundle.h"
#include "renderer/inf/model_loader.h"
#include "renderer/inf/uploader.h"

#include "assimp/impl/vertices/vertices_assimp.h"

namespace ark {
namespace plugin {
namespace assimp {

class ModelLoaderAssimp : public ModelLoader {
public:
    ModelLoaderAssimp(sp<Atlas> atlas, const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;
    virtual Model load(int32_t type) override;

//  [[plugin::resource-loader("assimp")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        SafePtr<Builder<Atlas>> _atlas;
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    class IndicesUploader : public Uploader {
    public:
        IndicesUploader(sp<ark::Array<Mesh>> meshes);

        virtual void upload(Writable& uploader) override;

    private:
        size_t calcIndicesSize(ark::Array<Mesh>& meshes) const;

    private:
        sp<ark::Array<Mesh>> _meshes;
    };

private:
    bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
    void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);

    sp<ModelBundle> makeModelBundle(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest, sp<Atlas> atlas);

    class Importer : public ModelBundle::Importer {
    public:
        Importer();

        virtual Model import(const String& src, const Rect& bounds) override;

    private:
        Mesh loadMesh(const aiMesh* mesh, const Rect& uvBounds, element_index_t indexOffset) const;
        Model loadModel(const aiScene* scene, const Rect& uvBounds) const;

        bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
        array<element_index_t> loadIndices(const aiMesh* mesh, element_index_t indexOffset) const;

    private:
        Assimp::Importer _importer;

    };

private:
    sp<ModelBundle> _models;
    std::vector<sp<Texture>> _textures;

};

}
}
}
#endif
