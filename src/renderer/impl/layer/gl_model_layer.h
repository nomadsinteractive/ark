#ifndef ARK_RENDERER_IMPL_LAYER_GL_MODEL_LAYER_H_
#define ARK_RENDERER_IMPL_LAYER_GL_MODEL_LAYER_H_

#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

#include "renderer/forwarding.h"

#include "platform/gl/gl.h"

namespace ark {

class GLModelLayer : public Layer {
public:
    GLModelLayer(const sp<GLModelLoader>& modelLoader, const sp<GLShader>& shader, const sp<Atlas>& atlas, const sp<ResourceLoaderContext>& resourceLoaderContext);

    const sp<Atlas>& atlas() const;

    virtual sp<RenderCommand> render(const Layer::Snapshot& layerContext, float x, float y) override;

//  [[plugin::resource-loader]]
    class BUILDER : public Builder<Layer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Layer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
        sp<Builder<Atlas>> _atlas;
        sp<Builder<GLModelLoader>> _model_loader;
        sp<Builder<GLShader>> _shader;
    };

private:
    sp<GLResourceManager> _resource_manager;
    sp<GLModelLoader> _model_loader;
    sp<GLShader> _shader;
    sp<Atlas> _atlas;

    sp<ResourceLoaderContext> _resource_loader_context;
    sp<GLShaderBindings> _shader_bindings;
};

}

#endif
