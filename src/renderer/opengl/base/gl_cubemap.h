#ifndef ARK_RENDERER_OPENGL_BASE_GL_CUBEMAP_H_
#define ARK_RENDERER_OPENGL_BASE_GL_CUBEMAP_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/opengl/base/gl_texture.h"

namespace ark {
namespace opengl {

class ARK_API GLCubemap : public GLTexture {
public:
    GLCubemap(sp<Recycler> recycler, sp<Size> size, sp<Texture::Parameters> parameters);

    virtual void clear(GraphicsContext& graphicsContext) override;
    virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    virtual void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;

};

}
}

#endif
