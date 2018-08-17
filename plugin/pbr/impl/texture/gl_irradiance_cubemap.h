#ifndef ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_IRRANDIANCE_CUBEMAP_H_
#define ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_IRRANDIANCE_CUBEMAP_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/gl_texture.h"

namespace ark {

class GLIrradianceCubemap : public GLTexture {
public:
    GLIrradianceCubemap(const sp<GLResourceManager>& resourceManager, Format format, Feature features, const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<Size>& size);

//  [[plugin::resource-loader("irradiance-cubemap")]]
    class BUILDER : public Builder<GLTexture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLTexture> build(const sp<Scope>& args) override;

    private:
        sp<GLResourceManager> _resource_manager;
        document _manifest;
        sp<Builder<Size>> _size;
        sp<Builder<GLShader>> _shader;
        sp<Builder<GLTexture>> _texture;
        GLTexture::Format _format;
        GLTexture::Feature _features;
    };

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) override;

private:
    sp<GLResourceManager> _resource_manager;
    sp<GLShader> _shader;
    sp<GLTexture> _texture;

};

}

#endif
