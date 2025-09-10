#include "renderer/base/texture_packer.h"

#include "core/ark.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/rect.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

TexturePacker::TexturePacker(sp<Texture> texture)
    : _texture(std::move(texture)), _channels(0), _bin_pack(texture ? texture->width() : 256, texture ? texture->height() : 256, false)
{
}

RectI TexturePacker::addBitmap(MaxRectsBinPack& binPack, sp<Bitmap> bitmap)
{
    sp<Variable<sp<Bitmap>>> bitmapProvider = sp<Variable<sp<Bitmap>>::Const>::make(bitmap);
    return addBitmap(binPack, std::move(bitmap), std::move(bitmapProvider));
}

RectI TexturePacker::addBitmap(MaxRectsBinPack& binPack, sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider)
{
    const MaxRectsBinPack::Rect rect = _bin_pack.Insert(static_cast<int32_t>(bounds->width()), static_cast<int32_t>(bounds->height()), MaxRectsBinPack::RectBestShortSideFit);
    if(rect.width == 0 || rect.height == 0)
    {
        resize(_bin_pack.width() * 2, _bin_pack.height() * 2);
        return addBitmap(binPack, std::move(bounds), std::move(bitmapProvider));
    }
    const RectI uv(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    addPackedBitmap(uv, std::move(bounds), std::move(bitmapProvider));
    return uv;
}

const Vector<TexturePacker::PackedBitmap>& TexturePacker::packedBitmaps() const
{
    return _packed_bitmaps;
}

void TexturePacker::addPackedBitmap(RectI uv, sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider)
{
    _channels = std::max(bounds->channels(), _channels);
    _packed_bitmaps.emplace_back(std::move(bounds), std::move(bitmapProvider), uv);
}

void TexturePacker::resize(const int32_t width, const int32_t height)
{
    Vector<PackedBitmap> packedBitmaps = std::move(_packed_bitmaps);
    _bin_pack = MaxRectsBinPack(width, height, false);
    for(auto& [bitmapBounds, bitmapProvider, uv] : packedBitmaps)
        addBitmap(_bin_pack, std::move(bitmapBounds), std::move(bitmapProvider));
}

void TexturePacker::updateTexture()
{
    if(_channels)
    {
        sp<Texture::Uploader> uploader = sp<Texture::Uploader>::make<PackedTextureUploader>(_texture->width(), _texture->height(), _channels, std::move(_packed_bitmaps));
        _texture->reset(*Ark::instance().renderController()->createTexture(_texture->size(), _texture->parameters(), std::move(uploader)));
    }
}

TexturePacker::PackedTextureUploader::PackedTextureUploader(uint32_t width, uint32_t height, uint8_t channels, std::vector<PackedBitmap> bitmaps)
    : _width(width), _height(height), _channels(channels), _bitmaps(std::move(bitmaps))
{
}

void TexturePacker::PackedTextureUploader::initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    const sp<Bitmap> content = sp<Bitmap>::make(_width, _height, _width * _channels, _channels, true);
    for(const auto& [bitmapBounds, bitmapProvider, uv] : _bitmaps)
    {
        const sp<Bitmap> s = bitmapProvider->val();
        content->draw(uv.left(), uv.top(), s->byteArray()->buf(), s->width(), s->height(), s->rowBytes());
    }
    delegate.uploadBitmap(graphicsContext, content, {content->byteArray()});
}

}
