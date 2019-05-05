#ifndef ARK_RENDERER_BASE_GL_TEXTURE_2D_H_
#define ARK_RENDERER_BASE_GL_TEXTURE_2D_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/opengl/base/gl_texture.h"

namespace ark {

class GLTexture2D : public GLTexture {
public:
    GLTexture2D(const sp<Recycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader);

    virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    virtual void uploadBitmap(GraphicsContext& graphicContext, uint32_t index, const Bitmap& bitmap) override;

};

}

#endif
