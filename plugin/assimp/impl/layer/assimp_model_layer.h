#ifndef ARK_PLUGIN_ASSIMP_IMPL_LAYER_ASSIMP_MODEL_LAYER_H_
#define ARK_PLUGIN_ASSIMP_IMPL_LAYER_ASSIMP_MODEL_LAYER_H_

#include <unordered_map>

#include <assimp/postprocess.h>
#include <assimp/version.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>

#include "core/base/object_pool.h"

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {
namespace plugin {
namespace assimp {

class AssimpModelLayer : public Layer {
public:
    AssimpModelLayer(const sp<GLShader>& shader, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RenderCommand> render(const Layer::Snapshot& layerContext, float x, float y) override;

//  [[plugin::resource-loader("assimp-model-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        document _manifest;
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<GLShader>> _shader;
    };

private:
    bytearray loadArrayBuffer(aiMesh* mesh, float scale) const;
    bytearray loadIndexBuffer(aiMesh* mesh) const;
    bitmap loadBitmap(const sp<ResourceLoaderContext>& resourceLoaderContext, aiTexture* tex) const;

private:
    sp<GLShader> _shader;
    sp<Assimp::Importer> _importer;
    std::unordered_map<int32_t, sp<aiScene>> _scences;
    GLBuffer _array_buffer;
    GLBuffer _index_buffer;
    sp<GLShaderBindings> _shader_bindings;
    ObjectPool _render_command_pool;
};

}
}
}

#endif
