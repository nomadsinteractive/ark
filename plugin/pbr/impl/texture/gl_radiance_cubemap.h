#ifndef ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_RANDIANCE_CUBEMAP_H_
#define ARK_PLUGIN_PBR_IMPL_TEXTURE_GL_RANDIANCE_CUBEMAP_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/opengl/base/gl_texture.h"

namespace ark {

class GLRadianceCubemap : public GLTexture {
public:
    GLRadianceCubemap(const sp<RenderController>& renderController, const sp<Texture::Parameters>& parameters, const sp<Texture>& texture, const sp<Size>& size);

//  [[plugin::resource-loader("radiance-cubemap")]]
    class BUILDER : public Builder<Texture> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture> build(const sp<Scope>& args) override;

    private:
        sp<RenderController> _render_controller;
        document _manifest;
        sp<Builder<Size>> _size;
        sp<Builder<Texture>> _texture;
        sp<Texture::Parameters> _parameters;
    };

    virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;

protected:
    virtual void doPrepareTexture(GraphicsContext& graphicsContext, uint32_t id) override;

private:
    sp<RenderController> _render_controller;
    sp<Texture> _texture;


};

}

#endif
