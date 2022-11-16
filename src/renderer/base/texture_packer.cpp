#include "renderer/base/texture_packer.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/rect.h"

#include "renderer/base/resource_loader_context.h"

#include "app/base/application_context.h"
#include "app/base/application_bundle.h"

namespace ark {

namespace {

class BitmapProvider : public Variable<bitmap> {
public:
    BitmapProvider(sp<BitmapLoaderBundle> bitmapBundle, String src)
        : _bitmap_bundle(std::move(bitmapBundle)), _src(std::move(src)) {

    }

    virtual bool update(uint64_t /*timestamp*/) override {
        return false;
    }

    virtual bitmap val() override {
        return _bitmap_bundle->get(_src);
    }

private:
    sp<BitmapLoaderBundle> _bitmap_bundle;
    String _src;
};

}

TexturePacker::TexturePacker(const ApplicationContext& applicationContext, sp<Texture> texture)
    : TexturePacker(applicationContext.applicationBundle()->bitmapBoundsBundle(), applicationContext.applicationBundle()->bitmapBundle(), applicationContext.renderController(), std::move(texture))
{
}

TexturePacker::TexturePacker(sp<BitmapLoaderBundle> bitmapBundleBounds, sp<BitmapLoaderBundle> bitmapBundle, sp<RenderController> renderController, sp<Texture> texture)
    : _bitmap_bundle_bounds(std::move(bitmapBundleBounds)), _bitmap_bundle(std::move(bitmapBundle)), _render_controller(std::move(renderController)), _texture(std::move(texture)), _channels(0)
{
}

RectI TexturePacker::addBitmap(MaxRectsBinPack& binPack, const String& src)
{
    return addBitmap(binPack, _bitmap_bundle_bounds->get(src), sp<BitmapProvider>::make(_bitmap_bundle, src));
}

RectI TexturePacker::addBitmap(MaxRectsBinPack& binPack, bitmap fragment)
{
    return addBitmap(binPack, fragment, sp<Variable<bitmap>::Const>::make(std::move(fragment)));
}

void TexturePacker::updateTexture()
{
    if(_channels)
    {
        sp<Texture::Uploader> uploader = sp<PackedTextureUploader>::make(_texture->width(), _texture->height(), _channels, std::move(_packed_bitmaps));
        const sp<Texture> tex = _render_controller->createTexture(_texture->size(), _texture->parameters(), std::move(uploader));
        _texture->setDelegate(tex->delegate());
    }
}

RectI TexturePacker::addBitmap(MaxRectsBinPack& binPack, const bitmap& bounds, sp<Variable<bitmap>> bitmapProvider)
{
    MaxRectsBinPack::Rect rect = binPack.Insert(static_cast<int32_t>(bounds->width()), static_cast<int32_t>(bounds->height()), MaxRectsBinPack::RectBestShortSideFit);
    DCHECK(rect.width != 0 && rect.height != 0, "Cannot insert more bitmap into the atlas(%d, %d), more space required", static_cast<int32_t>(_texture->width()), static_cast<int32_t>(_texture->height()));
    addPackedBitmap(rect.x, rect.y, bounds, std::move(bitmapProvider));
    return RectI(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

void TexturePacker::addPackedBitmap(int32_t x, int32_t y, const bitmap& bounds, sp<Variable<bitmap>> bitmapProvider)
{
    _channels = std::max(bounds->channels(), _channels);
    _packed_bitmaps.emplace_back(std::move(bitmapProvider), x, y);
}

TexturePacker::PackedTextureUploader::PackedTextureUploader(uint32_t width, uint32_t height, uint8_t channels, std::vector<PackedBitmap> bitmaps)
    : _width(width), _height(height), _channels(channels), _bitmaps(std::move(bitmaps))
{
}

void TexturePacker::PackedTextureUploader::initialize(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    const bitmap content = sp<Bitmap>::make(_width, _height, _width * _channels, _channels, true);
    for(const PackedBitmap& i : _bitmaps)
    {
        const bitmap s = i._bitmap_provider->val();
        content->draw(i._x, i._y, s->bytes()->buf(), s->width(), s->height(), s->rowBytes());
    }
    delegate.uploadBitmap(graphicsContext, content, {content->bytes()});
}

TexturePacker::PackedBitmap::PackedBitmap(sp<Variable<bitmap>> bitmapProvider, int32_t x, int32_t y)
    : _bitmap_provider(std::move(bitmapProvider)), _x(x), _y(y)
{
}

}
