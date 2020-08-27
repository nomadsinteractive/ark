#ifndef ARK_RENDERER_BASE_TEXTURE_PACKER_H_
#define ARK_RENDERER_BASE_TEXTURE_PACKER_H_

#include <vector>

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/forwarding.h"
#include "renderer/base/texture.h"

namespace ark {

class ARK_API TexturePacker {
public:
    TexturePacker(const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Texture> texture);

    RectI addBitmap(MaxRectsBinPack& binPack, const String& src);
    RectI addBitmap(MaxRectsBinPack& binPack, sp<Variable<bitmap>> bitmapProvider);
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

    class PackedTextureUploader : public Texture::Uploader {
    public:
        PackedTextureUploader(uint32_t width, uint32_t height, uint8_t channels, std::vector<PackedBitmap> bitmaps);

        void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate);

    private:
        uint32_t _width;
        uint32_t _height;
        uint8_t _channels;
        std::vector<PackedBitmap> _bitmaps;
    };

private:
    sp<ResourceLoaderContext> _resource_loader_context;
    sp<Texture> _texture;
    uint8_t _channels;

    std::vector<PackedBitmap> _packed_bitmaps;
};

}
#endif
