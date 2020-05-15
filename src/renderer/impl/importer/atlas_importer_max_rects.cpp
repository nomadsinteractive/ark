#include "renderer/impl/importer/atlas_importer_max_rects.h"

#include "core/util/math.h"
#include "core/util/documents.h"

#include "graphics/base/bitmap.h"
#include "graphics/base/bitmap_bundle.h"
#include "graphics/util/max_rects_bin_pack.h"

#include "renderer/base/atlas.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/base/texture.h"

namespace ark {

AtlasImporterMaxRects::AtlasImporterMaxRects(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

void AtlasImporterMaxRects::import(Atlas& atlas, BeanFactory& factory, const document& manifest)
{
    const sp<Texture>& texture = atlas.texture();
    MaxRectsBinPack binPack(texture->width(), texture->height(), false);
    std::vector<PackedBitmap> bitmaps;
    uint8_t channels = 0;
    for(const document& i : manifest->children())
    {
        const String& src = Documents::ensureAttribute(i, Constants::Attributes::SRC);
        const bitmap bitmap = _resource_loader_context->bitmapBoundsBundle()->get(src);
        channels = std::max(bitmap->channels(), channels);
        if(i->name() == Constants::Attributes::ATLAS)
        {
            Atlas imported(nullptr);
            const sp<Atlas::Importer> importer = factory.ensure<Atlas::Importer>(i, Scope());
            importer->import(imported, factory, i);
        }
        else
        {
            DCHECK(i->name() == "item", "No rule to import item \"%s\"", Documents::toString(i).c_str());
            int32_t type = Documents::ensureAttribute<int32_t>(i, Constants::Attributes::TYPE);
            MaxRectsBinPack::Rect rect = binPack.Insert(bitmap->width(), bitmap->height(), MaxRectsBinPack::RectBestShortSideFit);
            Rect bounds(0, 0, 1.0f, 1.0f);
            atlas.add(type, rect.x, rect.y, rect.x + rect.width, rect.y + rect.height, bounds, V2(rect.width, rect.height), V2(0.5f, 0.5f));
            bitmaps.emplace_back<PackedBitmap>({src, rect.x, rect.y});
        }
    }
    const sp<Texture::Uploader> uploader = sp<MaxRectsTextureUploader>::make(texture->width(),texture->height(), channels, _resource_loader_context->bitmapBundle(), std::move(bitmaps));
    const sp<Texture> tex = _resource_loader_context->renderController()->createTexture(texture->size(), texture->parameters(), uploader);
    texture->setDelegate(tex->delegate());
}

AtlasImporterMaxRects::BUILDER::BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext)
{
}

sp<Atlas::Importer> AtlasImporterMaxRects::BUILDER::build(const Scope& /*args*/)
{
    return sp<AtlasImporterMaxRects>::make(_resource_loader_context);
}

AtlasImporterMaxRects::MaxRectsTextureUploader::MaxRectsTextureUploader(uint32_t width, uint32_t height, uint8_t channels, const sp<BitmapBundle>& bitmapBundle, std::vector<PackedBitmap> bitmaps)
    : _width(width), _height(height), _channels(channels), _bitmap_bundle(bitmapBundle), _bitmaps(std::move(bitmaps))
{
}

void AtlasImporterMaxRects::MaxRectsTextureUploader::upload(GraphicsContext& graphicsContext, Texture::Delegate& delegate)
{
    const bitmap content = sp<Bitmap>::make(_width, _height, _width * _channels, _channels, true);
    for(const PackedBitmap& i : _bitmaps)
    {
        const bitmap s = _bitmap_bundle->get(i._src);
        content->draw(s->bytes()->buf(), s->width(), s->height(), i._x, i._y, s->rowBytes());
    }
    delegate.uploadBitmap(graphicsContext, 0, content);
}

}
