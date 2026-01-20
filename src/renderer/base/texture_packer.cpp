#include "renderer/base/texture_packer.h"

#include "core/ark.h"
#include "core/inf/array.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/rect.h"
#include "graphics/components/size.h"

#include "renderer/base/render_controller.h"

namespace ark {

namespace {

class PackedTextureUploader final : public Texture::Uploader {
public:
    PackedTextureUploader(sp<Size> size, const uint8_t channels, Vector<TexturePacker::PackedBitmap> bitmaps)
        : _size(std::move(size)), _channels(channels), _bitmaps(std::move(bitmaps))
    {
    }

    void initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate) override
    {
        const V3 s = _size->val();
        const uint32_t width = static_cast<uint32_t>(s.x());
        const uint32_t height = static_cast<uint32_t>(s.y());
        const Bitmap content(width, height, width * _channels, _channels, true);
        for(const auto& [name, bitmapBounds, bitmapProvider, uv] : _bitmaps)
        {
            const sp<Bitmap> bitmap = bitmapProvider->val();
            content.draw(uv.left(), uv.top(), bitmap->byteArray()->buf(), bitmap->width(), bitmap->height(), bitmap->rowBytes());
        }
        delegate.uploadBitmap(graphicsContext, content, {content.byteArray()});
    }

private:
    sp<Size> _size;
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

int32_t TexturePacker::width() const
{
    return _bin_pack.width();
}

int32_t TexturePacker::height() const
{
    return _bin_pack.height();
}

void TexturePacker::addBitmap(sp<Bitmap> bitmap, String name)
{
    sp<Variable<sp<Bitmap>>> bitmapProvider = sp<Variable<sp<Bitmap>>::Const>::make(bitmap);
    addBitmap(std::move(bitmap), std::move(bitmapProvider), std::move(name));
}

void TexturePacker::addBitmap(sp<Bitmap> bounds, sp<Variable<bitmap>> bitmapProvider, String name)
{
    const MaxRectsBinPack::Rect rect = _bin_pack.Insert(static_cast<int32_t>(bounds->width()), static_cast<int32_t>(bounds->height()), MaxRectsBinPack::RectBestShortSideFit);
    if(rect.width == 0 || rect.height == 0)
    {
        resize(_bin_pack.width() * 2, _bin_pack.height() * 2);
        return addBitmap(std::move(bounds), std::move(bitmapProvider), std::move(name));
    }
    const RectI uv(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
    addPackedBitmap(uv, std::move(bounds), std::move(bitmapProvider), std::move(name));
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

sp<Texture> TexturePacker::createTexture(sp<Size> size, sp<Texture::Parameters> parameters) const
{
    if(!_channels)
        return nullptr;

    sp<Size> s = size ? std::move(size) : sp<Size>::make(_bin_pack.width(), _bin_pack.height());
    sp<Texture::Uploader> uploader = sp<Texture::Uploader>::make<PackedTextureUploader>(s, _channels, _packed_bitmaps);
    return Ark::instance().renderController()->createTexture(std::move(s), parameters ? std::move(parameters) : sp<Texture::Parameters>::make(Texture::TYPE_2D, nullptr, toTextureFormat(_channels)), std::move(uploader));
}

void TexturePacker::updateTexture(Texture& texture, sp<Size> size) const
{
    if(_channels)
        texture.reset(*createTexture(std::move(size), texture.parameters()));
}

}
