#pragma once

#include <vector>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API TexturePacker {
public:
    TexturePacker(const ApplicationContext& applicationContext, sp<Texture> texture);
    TexturePacker(sp<BitmapLoaderBundle> bitmapBundleBounds, sp<BitmapLoaderBundle> bitmapBundle, sp<RenderController> renderController, sp<Texture> texture);

    RectI addBitmap(MaxRectsBinPack& binPack, const String& src);
    RectI addBitmap(MaxRectsBinPack& binPack, bitmap fragment);
    RectI addBitmap(MaxRectsBinPack& binPack, const bitmap& bounds, sp<Variable<bitmap>> bitmapProvider);

    void addPackedBitmap(int32_t x, int32_t y, const bitmap& bounds, sp<Variable<bitmap>> bitmapProvider);

    void updateTexture();

private:
    struct PackedBitmap {
        PackedBitmap(sp<Variable<bitmap>> bitmapProvider, int32_t x, int32_t y);

        sp<Variable<bitmap>> _bitmap_provider;
        int32_t _x;
        int32_t _y;
    };

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

private:
    sp<BitmapLoaderBundle> _bitmap_bundle_bounds;
    sp<BitmapLoaderBundle> _bitmap_bundle;
    sp<RenderController> _render_controller;
    sp<Texture> _texture;
    uint8_t _channels;

    Vector<PackedBitmap> _packed_bitmaps;
};

}
