#ifndef ARK_RENDERER_OPENGL_ES30_GL_RESOURCE_GL_DYNAMIC_CUBEMAP_H_
#define ARK_RENDERER_OPENGL_ES30_GL_RESOURCE_GL_DYNAMIC_CUBEMAP_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/texture.h"

namespace ark {

class GLDynamicCubemap : public Texture {
public:
    GLDynamicCubemap(const sp<GLResourceManager>& resourceManager, Format format, Feature features, const sp<Shader>& shader, const sp<Texture>& texture, const sp<Size>& size);

//  [[plugin::resource-loader("dynamic-cubemap")]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<GLResourceManager> _resource_manager;
        document _manifest;
        sp<Builder<Size>> _size;
        sp<Builder<Shader>> _shader;
        sp<Builder<Texture>> _texture;
        Texture::Format _format;
        Texture::Feature _features;
    };

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) override;

private:
    sp<GLResourceManager> _resource_manager;
    sp<Shader> _shader;
    sp<Texture> _texture;

};

}

#endif
