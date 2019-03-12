#ifndef ARK_RENDERER_OPENGL_ES30_LAYER_PARTICLE_LAYER_H_
#define ARK_RENDERER_OPENGL_ES30_LAYER_PARTICLE_LAYER_H_

#include "core/base/memory_pool.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_layer.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {
namespace gles30 {
/*
class ParticleLayer : public Layer {
public:
    ParticleLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RenderCommand> render(const Layer::Snapshot& layerContext, float x, float y) override;

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
    sp<ShaderBindings> _shader_bindings;

    GLBuffer::Snapshot _index_buffer;
    GLBuffer _transform_array_buffer;

    MemoryPool _memory_pool;
};
*/
}
}

#endif
