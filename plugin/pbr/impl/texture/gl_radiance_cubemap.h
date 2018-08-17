#ifndef ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_RANDIANCE_CUBEMAP_H_
#define ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_RANDIANCE_CUBEMAP_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/gl_texture.h"

namespace ark {

class GLRadianceCubemap : public GLTexture {
public:
    GLRadianceCubemap(const sp<GLResourceManager>& resourceManager, Format format, Feature features, const sp<GLTexture>& texture, const sp<Size>& size);

//  [[plugin::resource-loader("radiance-cubemap")]]
    class BUILDER : public Builder<GLTexture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLTexture> build(const sp<Scope>& args) override;

    private:
        sp<GLResourceManager> _resource_manager;
        document _manifest;
        sp<Builder<Size>> _size;
        sp<Builder<GLTexture>> _texture;
        GLTexture::Format _format;
        GLTexture::Feature _features;
    };

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) override;

private:
    sp<GLResourceManager> _resource_manager;
    sp<GLTexture> _texture;

};

}

#endif
