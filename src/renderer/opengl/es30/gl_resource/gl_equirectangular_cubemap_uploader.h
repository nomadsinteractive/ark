#ifndef ARK_RENDERER_OPENGL_ES30_GL_EQUIRECTANGULAR_CUBEMAP_UPLOADER_H_
#define ARK_RENDERER_OPENGL_ES30_GL_EQUIRECTANGULAR_CUBEMAP_UPLOADER_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/base/texture.h"

namespace ark {

class GLEquirectangularCubemapUploader : public Texture::Uploader {
public:
    GLEquirectangularCubemapUploader(const sp<RenderController>& renderController, const Shader& shader, const sp<Texture>& texture, const sp<Size>& size);

    virtual void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override;

//  [[plugin::resource-loader("equirectangular")]]
    class BUILDER : public Builder<Texture::Uploader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Texture::Uploader> build(const Scope& args) override;

    private:
        sp<RenderController> _render_controller;
        sp<Builder<Size>> _size;
        sp<Builder<Shader>> _shader;
        sp<Builder<Texture>> _texture;
    };

private:
    sp<RenderController> _render_controller;
    sp<ShaderBindings> _shader_bindings;
    sp<Texture> _texture;
    sp<Size> _size;

};

}

#endif
