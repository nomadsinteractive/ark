#include "renderer/base/texture_packer.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/base/rect.h"

#include "renderer/base/resource_loader_context.h"

namespace ark {

namespace {

class BitmapProvider : public Variable<bitmap> {
public:
    BitmapProvider(sp<BitmapBundle> bitmapBundle, String src)
        : _bitmap_bundle(std::move(bitmapBundle)), _src(std::move(src)) {

    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

    virtual bitmap val() override {
        return _bitmap_bundle->get(_src);
    }

private:
    sp<BitmapBundle> _bitmap_bundle;
    String _src;
};

}

TexturePacker::TexturePacker(const sp<ResourceLoaderContext>& resourceLoaderContext, sp<Texture> texture, bool allowRotate)
    : _resource_loader_context(resourceLoaderContext), _texture(std::move(texture)), _max_rects_bin_pack(static_cast<int32_t>(_texture->width()), static_cast<int32_t>(_texture->height()), allowRotate),
      _channels(0)
{
}

RectI TexturePacker::addBitmap(const String& src)
{
    return doAddBitmap(_resource_loader_context->bitmapBoundsBundle()->get(src), sp<BitmapProvider>::make(_resource_loader_context->bitmapBundle(), src));
}

RectI TexturePacker::addBitmap(const bitmap& bitmap)
{
    return doAddBitmap(bitmap, sp<Variable<sp<Bitmap>>::Const>::make(bitmap));
}

RectI TexturePacker::addBitmap(sp<Variable<bitmap>> bitmapProvider)
{
    const bitmap bitmap = bitmapProvider->val();
    return doAddBitmap(bitmap, std::move(bitmapProvider));
}

void TexturePacker::updateTexture()
{
    DCHECK(_channels, "No bitmap was added");
    const sp<Texture::Uploader> uploader = sp<PackedTextureUploader>::make(_texture->width(), _texture->height(), _channels, std::move(_packed_bitmaps));
    const sp<Texture> tex = _resource_loader_context->renderController()->createTexture(_texture->size(), _texture->parameters(), uploader);
    _texture->setDelegate(tex->delegate());
}

RectI TexturePacker::doAddBitmap(const bitmap& bounds, sp<Variable<bitmap>> bitmapProvider)
{
    _channels = std::max(bounds->channels(), _channels);

    MaxRectsBinPack::Rect rect = _max_rects_bin_pack.Insert(static_cast<int32_t>(bounds->width()), static_cast<int32_t>(bounds->height()), MaxRectsBinPack::RectBestShortSideFit);
    DCHECK(rect.width != 0 && rect.height != 0, "Cannot insert more bitmap into the atlas(%d, %d), more space required", static_cast<int32_t>(_texture->width()), static_cast<int32_t>(_texture->height()));
    _packed_bitmaps.emplace_back(std::move(bitmapProvider), rect.x, rect.y);
    return RectI(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

TexturePacker::PackedTextureUploader::PackedTextureUploader(uint32_t width, uint32_t height, uint8_t channels, std::vector<PackedBitmap> bitmaps)
    : _width(width), _height(height), _channels(channels), _bitmaps(std::move(bitmaps))
{
}

void TexturePacker::PackedTextureUploader::upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    const bitmap content = sp<Bitmap>::make(_width, _height, _width * _channels, _channels, true);
    for(const PackedBitmap& i : _bitmaps)
    {
        const bitmap s = i._bitmap_provider->val();
        content->draw(s->bytes()->buf(), s->width(), s->height(), i._x, i._y, s->rowBytes());
    }
    delegate.uploadBitmap(graphicsContext, content, {content->bytes()});
}

TexturePacker::PackedBitmap::PackedBitmap(sp<Variable<bitmap>> bitmapProvider, int32_t x, int32_t y)
    : _bitmap_provider(std::move(bitmapProvider)), _x(x), _y(y)
{
}

}
