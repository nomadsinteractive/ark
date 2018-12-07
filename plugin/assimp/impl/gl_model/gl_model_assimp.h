#ifndef ARK_PLUGIN_ASSIMP_IMPL_GL_MODEL_GL_MODEL_ASSIMP_H_
#define ARK_PLUGIN_ASSIMP_IMPL_GL_MODEL_GL_MODEL_ASSIMP_H_

#include <unordered_map>

#include <assimp/postprocess.h>
#include <assimp/version.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

#include "core/forwarding.h"
#include "core/inf/builder.h"

#include "graphics/base/layer.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"

namespace ark {
namespace plugin {
namespace assimp {

class GLModelAssimp : public RenderModel {
public:
    GLModelAssimp(const sp<ResourceLoaderContext>& resourceLoaderContext, const document& manifest);

    virtual void initialize(ShaderBindings& bindings) override;
    virtual void start(ModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) override;
    virtual void load(ModelBuffer& buf, int32_t type, const V& scale) override;

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
    struct UV {
        UV();
        UV(uint16_t u, uint16_t v);

        uint16_t u;
        uint16_t v;
    };

    class Mesh {
    public:
        Mesh(const aiMesh* mesh);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Mesh);

        void load(ModelBuffer& buf) const;

    private:
        bytearray loadIndices(const aiMesh* mesh) const;

    private:
        array<V3> _vertices;
        array<V3> _normals;
        array<std::pair<V3, V3>> _tangents;
        array<UV> _uvs;

        bytearray _indices;
        bitmap _texture;
    };

private:
    bitmap loadBitmap(const sp<ImageResource>& imageResource, const aiTexture* tex) const;

private:
    sp<Assimp::Importer> _importer;
    std::unordered_map<int32_t, sp<Mesh>> _models;

};

}
}
}
#endif
