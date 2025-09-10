#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API TexturePacker {
public:
    TexturePacker(sp<Texture> texture);

    struct PackedBitmap {
        sp<Bitmap> _bitmap_bounds;
        sp<Variable<sp<Bitmap>>> _bitmap_provider;
        RectI _uv;
    };

    RectI addBitmap(MaxRectsBinPack& binPack, sp<Bitmap> bitmap);
    RectI addBitmap(MaxRectsBinPack& binPack, sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider);

    const Vector<PackedBitmap>& packedBitmaps() const;

    void updateTexture();

private:
    class PackedTextureUploader final : public Texture::Uploader {
    public:
        PackedTextureUploader(uint32_t width, uint32_t height, uint8_t channels, Vector<PackedBitmap> bitmaps);

        void initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override;

    private:
        uint32_t _width;
        uint32_t _height;
        uint8_t _channels;
        Vector<PackedBitmap> _bitmaps;
    };

    void addPackedBitmap(RectI uv, sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider);
    void resize(int32_t width, int32_t height);

private:
    sp<Texture> _texture;
    uint8_t _channels;

    MaxRectsBinPack _bin_pack;
    Vector<PackedBitmap> _packed_bitmaps;
};

}
