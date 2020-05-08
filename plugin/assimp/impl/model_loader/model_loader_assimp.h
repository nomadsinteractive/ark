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
#include "renderer/inf/model_loader.h"

namespace ark {
namespace plugin {
namespace assimp {

class ModelLoaderAssimp : public ModelLoader {
public:
    ModelLoaderAssimp(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest);

    virtual sp<RenderCommandComposer> makeRenderCommandComposer() override;

    virtual void initialize(ShaderBindings& shaderBindings) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;
    virtual Model load(int32_t type) override;

//  [[plugin::resource-loader("assimp")]]
    class BUILDER : public Builder<ModelLoader> {
    public:
        BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<ModelLoader> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        document _manifest;
    };

private:
    void loadSceneTexture(const ResourceLoaderContext& renderController, const aiTexture* tex);

    array<element_index_t> loadIndices(const aiMesh* mesh) const;

private:
    struct Stub {
        Stub();

        void initialize(const document& manifest, const ResourceLoaderContext& resourceLoaderContext);

        Assimp::Importer _importer;
        sp<MultiModels> _models;
        std::vector<sp<Texture>> _textures;

    private:
        Model loadModel(const aiMesh* mesh) const;

        bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;
        array<element_index_t> loadIndices(const aiMesh* mesh) const;

        void loadSceneTexture(const ResourceLoaderContext& resourceLoaderContext, const aiTexture* tex);

    };

private:
    sp<Stub> _stub;
};

}
}
}
#endif
