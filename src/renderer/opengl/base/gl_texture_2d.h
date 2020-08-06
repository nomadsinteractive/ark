#ifndef ARK_RENDERER_BASE_GL_TEXTURE_2D_H_
#define ARK_RENDERER_BASE_GL_TEXTURE_2D_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/opengl/base/gl_texture.h"

namespace ark {
namespace opengl {

class GLTexture2D : public GLTexture {
public:
    GLTexture2D(const sp<Recycler>& recycler, const sp<Size>& size, const sp<Texture::Parameters>& parameters, const sp<Texture::Uploader>& uploader);

    virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    virtual void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;

};

}
}

#endif
