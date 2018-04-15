#ifndef ARK_RENDERER_IMPL_LAYER_GL_MODEL_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_GL_MODEL_LAYER_H_

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

#include "renderer/forwarding.h"

#include "platform/gl/gl.h"

namespace ark {

class GLModelLayer : public Layer {
public:
    GLModelLayer(const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<GLModelLoader>& modelLoader, GLenum mode, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual sp<RenderCommand> render(const LayerContext::Snapshot& layerContext, float x, float y) override;

//  [[plugin::resource-loader("model-layer")]]
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
    sp<GLModelLoader> _model_loader;
    sp<GLResourceManager> _resource_manager;
    sp<GLShader> _shader;
    GLenum _mode;

    sp<ObjectPool> _render_command_pool;
    sp<MemoryPool> _memory_pool;
    sp<GLShaderBindings> _shader_bindings;
};

}

#endif
