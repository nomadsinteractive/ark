#include "renderer/base/texture_packer.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/base/rect.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

TexturePacker::TexturePacker(const sp<ResourceLoaderContext>& resourceLoaderContext, int32_t width, int32_t height, bool allowRotate)
    : _resource_loader_context(resourceLoaderContext), _max_rects_bin_pack(width, height, allowRotate), _width(width), _height(height), _channels(0)
{
}

RectI TexturePacker::addBitmap(const String& src)
{
    const bitmap bitmap = _resource_loader_context->bitmapBoundsBundle()->get(src);
    _channels = std::max(bitmap->channels(), _channels);

    MaxRectsBinPack::Rect rect = _max_rects_bin_pack.Insert(bitmap->width(), bitmap->height(), MaxRectsBinPack::RectBestShortSideFit);
    DCHECK(rect.width != 0 && rect.height != 0, "Cannot insert more bitmap into the atlas(%d, %d), more space required", _width, _height);
    _packed_bitmaps.emplace_back<PackedBitmap>({src, rect.x, rect.y});
    return RectI(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

void TexturePacker::updateTexture(Texture& texture)
{
    DCHECK(_channels, "No bitmap was added");
    const sp<Texture::Uploader> uploader = sp<MaxRectsTextureUploader>::make(texture.width(),texture.height(), _channels, _resource_loader_context->bitmapBundle(), std::move(_packed_bitmaps));
    const sp<Texture> tex = _resource_loader_context->renderController()->createTexture(texture.size(), texture.parameters(), uploader);
    texture.setDelegate(tex->delegate());
}

TexturePacker::MaxRectsTextureUploader::MaxRectsTextureUploader(uint32_t width, uint32_t height, uint8_t channels, const sp<BitmapBundle>& bitmapBundle, std::vector<PackedBitmap> bitmaps)
    : _width(width), _height(height), _channels(channels), _bitmap_bundle(bitmapBundle), _bitmaps(std::move(bitmaps))
{
}

void TexturePacker::MaxRectsTextureUploader::upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    const bitmap content = sp<Bitmap>::make(_width, _height, _width * _channels, _channels, true);
    for(const PackedBitmap& i : _bitmaps)
    {
        const bitmap s = _bitmap_bundle->get(i._src);
        content->draw(s->bytes()->buf(), s->width(), s->height(), i._x, i._y, s->rowBytes());
    }
    delegate.uploadBitmap(graphicsContext, content, {content->bytes()});
}

}
