#include "renderer/base/texture_packer.h"

#include "core/ark.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/rect.h"
#include "graphics/components/size.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

namespace {

class PackedTextureUploader final : public Texture::Uploader {
public:
    PackedTextureUploader(const uint32_t width, const uint32_t height, const uint8_t channels, Vector<TexturePacker::PackedBitmap> bitmaps)
        : _width(width), _height(height), _channels(channels), _bitmaps(std::move(bitmaps))
    {
    }

    void initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override
    {
        const sp<Bitmap> content = sp<Bitmap>::make(_width, _height, _width * _channels, _channels, true);
        for(const auto& [name, bitmapBounds, bitmapProvider, uv] : _bitmaps)
        {
            const sp<Bitmap> s = bitmapProvider->val();
            content->draw(uv.left(), uv.top(), s->byteArray()->buf(), s->width(), s->height(), s->rowBytes());
        }
        delegate.uploadBitmap(graphicsContext, content, {content->byteArray()});
    }

private:
    uint32_t _width;
    uint32_t _height;
    uint8_t _channels;
    Vector<TexturePacker::PackedBitmap> _bitmaps;
};

Texture::Format toTextureFormat(const uint8_t channels)
{
    if(channels == 1)
        return Texture::FORMAT_R;
    if(channels == 2)
        return Texture::FORMAT_RG;
    if(channels == 3)
        return Texture::FORMAT_RGB;
    CHECK(channels == 4, "Wrong channels: %d", channels);
    return Texture::FORMAT_RGBA;

}

}

TexturePacker::TexturePacker(const int32_t initialWidth, const int32_t initialHeight)
    : _channels(0), _bin_pack(initialWidth, initialHeight, false)
{
}

RectI TexturePacker::addBitmap(sp<Bitmap> bitmap, String name)
{
    sp<Variable<sp<Bitmap>>> bitmapProvider = sp<Variable<sp<Bitmap>>::Const>::make(bitmap);
    return addBitmap(std::move(bitmap), std::move(bitmapProvider), std::move(name));
}

RectI TexturePacker::addBitmap(sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider, String name)
{
    const MaxRectsBinPack::Rect rect = _bin_pack.Insert(static_cast<int32_t>(bounds->width()), static_cast<int32_t>(bounds->height()), MaxRectsBinPack::RectBestShortSideFit);
    if(rect.width == 0 || rect.height == 0)
    {
        resize(_bin_pack.width() * 2, _bin_pack.height() * 2);
        return addBitmap(std::move(bounds), std::move(bitmapProvider));
    }
    const RectI uv(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    addPackedBitmap(uv, std::move(bounds), std::move(bitmapProvider), std::move(name));
    return uv;
}

const Vector<TexturePacker::PackedBitmap>& TexturePacker::packedBitmaps() const
{
    return _packed_bitmaps;
}

void TexturePacker::addPackedBitmap(RectI uv, sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider, String name)
{
    _channels = std::max(bounds->channels(), _channels);
    _packed_bitmaps.emplace_back(std::move(name), std::move(bounds), std::move(bitmapProvider), uv);
}

void TexturePacker::resize(const int32_t width, const int32_t height)
{
    Vector<PackedBitmap> packedBitmaps = std::move(_packed_bitmaps);
    _bin_pack = MaxRectsBinPack(width, height, false);
    for(auto& [name, bitmapBounds, bitmapProvider, uv] : packedBitmaps)
        addBitmap(std::move(bitmapBounds), std::move(bitmapProvider), std::move(name));
}

sp<Texture> TexturePacker::createTexture(sp<Texture::Parameters> parameters) const
{
    if(_channels)
        return nullptr;

    sp<Texture::Uploader> uploader = sp<Texture::Uploader>::make<PackedTextureUploader>(_bin_pack.width(), _bin_pack.height(), _channels, _packed_bitmaps);
    return Ark::instance().renderController()->createTexture(sp<Size>::make(_bin_pack.width(), _bin_pack.height()), parameters ? std::move(parameters) : sp<Texture::Parameters>::make(Texture::TYPE_2D, nullptr, toTextureFormat(_channels)), std::move(uploader));
}

void TexturePacker::updateTexture(Texture& texture)
{
    if(_channels)
    {
        sp<Texture::Uploader> uploader = sp<Texture::Uploader>::make<PackedTextureUploader>(_bin_pack.width(), _bin_pack.height(), _channels, _packed_bitmaps);
        texture.reset(*createTexture(texture.parameters()));
    }
}

}
