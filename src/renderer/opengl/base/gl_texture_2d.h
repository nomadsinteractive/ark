#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "renderer/opengl/base/gl_texture.h"

namespace ark::opengl {

class GLTexture2D final : public GLTexture {
public:
    GLTexture2D(sp<Recycler> recycler, sp<Size> size, sp<Texture::Parameters> parameters);
    ~GLTexture2D() override;

    virtual bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    virtual void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;
};

}
