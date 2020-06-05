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
    TexturePacker(const sp<ResourceLoaderContext>& resourceLoaderContext, int32_t width, int32_t height, bool allowRotate);

    RectI addBitmap(const String& src);
    void updateTexture(Texture& texture);

private:
    struct PackedBitmap {
        String _src;
        int32_t _x;
        int32_t _y;
    };

    class MaxRectsTextureUploader : public Texture::Uploader {
    public:
        MaxRectsTextureUploader(uint32_t width, uint32_t height, uint8_t channels, const sp<BitmapBundle>& bitmapBundle, std::vector<PackedBitmap> bitmaps);

        void upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate);

    private:
        uint32_t _width;
        uint32_t _height;
        uint8_t _channels;
        sp<BitmapBundle> _bitmap_bundle;
        std::vector<PackedBitmap> _bitmaps;

    };

private:
    sp<ResourceLoaderContext> _resource_loader_context;
    MaxRectsBinPack _max_rects_bin_pack;

    int32_t _width;
    int32_t _height;
    uint8_t _channels;

    std::vector<PackedBitmap> _packed_bitmaps;
};

}
#endif
