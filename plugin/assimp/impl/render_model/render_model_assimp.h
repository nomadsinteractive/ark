#ifndef ARK_PLUGIN_ASSIMP_IMPL_RENDER_MODEL_RENDER_MODEL_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_RENDER_MODEL_RENDER_MODEL_ASSIMP_H_

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
#include "renderer/inf/render_model.h"

namespace ark {
namespace plugin {
namespace assimp {

class RenderModelAssimp : public RenderModel {
public:
    RenderModelAssimp(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest);

    virtual sp<ShaderBindings> makeShaderBindings(const Shader& shader) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual void start(DrawingBuffer& buf, const RenderLayer::Snapshot& snapshot) override;
    virtual void load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot) override;

//  [[plugin::resource-loader("assimp")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        document _manifest;
    };

private:
    void loadSceneTexture(const ResourceLoaderContext& renderController, const aiTexture* tex);

    bitmap loadBitmap(const sp<BitmapBundle>& imageResource, const aiTexture* tex) const;

    sp<Model> loadModel(const aiMesh* mesh);
    array<element_index_t> loadIndices(const aiMesh* mesh) const;

    void compose(const Model& model, int32_t modelId, DrawingBuffer& buf) const;

private:
    sp<Assimp::Importer> _importer;
    std::unordered_map<int32_t, sp<Model>> _models;
    std::vector<sp<Texture>> _textures;
    array<Matrix> _model_matrics;

};

}
}
}
#endif
