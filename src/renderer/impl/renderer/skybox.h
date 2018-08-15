#ifndef ARK_RENDERER_IMPL_RENDERER_SKYBOX_H_
#define ARK_RENDERER_IMPL_RENDERER_SKYBOX_H_

#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {

class Skybox : public Renderer, public Block {
public:
    Skybox(const sp<Size>& size, const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const sp<Size>& size() override;

//  [[plugin::resource-loader("skybox")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Size>> _size;
        sp<Builder<GLShader>> _shader;
        sp<Builder<GLTexture>> _texture;
    };

private:
    sp<Size> _size;
    sp<GLResourceManager> _resource_manager;
    sp<GLShader> _shader;

    sp<GLShaderBindings> _shader_bindings;
    sp<ObjectPool> _object_pool;
};

}

#endif
