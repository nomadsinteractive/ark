#pragma once

#include <bgfx/bgfx.h>

#include "renderer/base/texture.h"

#include "bgfx/base/resource_base.h"

namespace ark::plugin::bgfx {

class TextureBgfx final : public ResourceBase<::bgfx::TextureHandle, Texture::Delegate> {
public:
    TextureBgfx(Texture::Type type, uint32_t width, uint32_t height, sp<Texture::Parameters> parameters);

    void upload(GraphicsContext& graphicsContext, const sp<Texture::Uploader>& uploader) override;
    void clear(GraphicsContext& graphicsContext) override;

    bool download(GraphicsContext& graphicsContext, Bitmap& bitmap) override;
    void uploadBitmap(GraphicsContext& graphicsContext, const Bitmap& bitmap, const std::vector<sp<ByteArray>>& imagedata) override;

    const sp<Texture::Parameters>& parameters() const;

private:
    uint32_t _width;
    uint32_t _height;
    sp<Texture::Parameters> _parameters;
};

}
