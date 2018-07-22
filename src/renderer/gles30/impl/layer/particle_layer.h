#ifndef ARK_RENDERER_GLES30_IMPL_LAYER_PARTICLE_LAYER_H_
#define ARK_RENDERER_GLES30_IMPL_LAYER_PARTICLE_LAYER_H_

#include "core/base/memory_pool.h"

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {
namespace gles30 {

class ParticleLayer : public Layer {
public:
    ParticleLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RenderCommand> render(const LayerContext::Snapshot& layerContext, float x, float y) override;

//  [[plugin::resource-loader("particle-layer")]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLShader>> _shader;
    };

private:
    sp<Atlas> _atlas;

    sp<ResourceLoaderContext> _resource_loader_context;
    sp<GLShaderBindings> _shader_bindings;

    GLBuffer::Snapshot _index_buffer;
    GLBuffer _transform_array_buffer;

    MemoryPool _memory_pool;
};

}
}

#endif
