#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "opengl/base/gl_texture.h"

namespace ark::plugin::opengl {

class GLCubemap final : public GLTexture {
public:
    GLCubemap(sp<Recycler> recycler, sp<Size> size, sp<Texture::Parameters> parameters);

    void clear(GraphicsContext& graphicsContext) override;
    bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;
};

}
