#pragma once

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/rect.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"


namespace ark {

class ARK_API TexturePacker {
public:
    TexturePacker(int32_t initialWidth, int32_t initialHeight);

    struct PackedBitmap {
        String _name;
        sp<Bitmap> _bitmap_bounds;
        sp<Variable<sp<Bitmap>>> _bitmap_provider;
        RectI _uv;
    };

    int32_t width() const;
    int32_t height() const;

    void addBitmap(sp<Bitmap> bitmap, String name = "");
    void addBitmap(sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider, String name = "");

    const Vector<PackedBitmap>& packedBitmaps() const;

    sp<Texture> createTexture(sp<Size> size = nullptr, sp<Texture::Parameters> parameters = nullptr) const;
    void updateTexture(Texture& texture, sp<Size> size = nullptr) const;

private:
    void addPackedBitmap(RectI uv, sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider, String name);
    void resize(int32_t width, int32_t height);

private:
    uint8_t _channels;

    MaxRectsBinPack _bin_pack;
    Vector<PackedBitmap> _packed_bitmaps;
};

}
