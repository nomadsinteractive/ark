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
    TexturePacker(int32_t initialWidth, int32_t initialHeight);

    struct PackedBitmap {
        String _name;
        sp<Bitmap> _bitmap_bounds;
        sp<Variable<sp<Bitmap>>> _bitmap_provider;
        RectI _uv;
    };

    RectI addBitmap(sp<Bitmap> bitmap, String name = "");
    RectI addBitmap(sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider, String name = "");

    const Vector<PackedBitmap>& packedBitmaps() const;

    sp<Texture> createTexture(sp<Texture::Parameters> parameters = nullptr) const;
    void updateTexture(Texture& texture);

private:
    void addPackedBitmap(RectI uv, sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider, String name);
    void resize(int32_t width, int32_t height);

private:
    uint8_t _channels;

    MaxRectsBinPack _bin_pack;
    Vector<PackedBitmap> _packed_bitmaps;
};

}
