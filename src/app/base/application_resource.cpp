#include "app/base/application_resource.h"

#include "core/base/string.h"
#include "core/inf/dictionary.h"

#include "graphics/base/image_asset.h"
#include "graphics/impl/bitmap_loader/jpeg_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/png_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/stb_bitmap_loader.h"

#include "renderer/base/resource_manager.h"

namespace ark {

ApplicationResource::ApplicationResource(const sp<Dictionary<document>>& documents, const sp<Asset>& images)
    : _images(images), _documents(documents), _bitmap_loader(createImageLoader(false)), _bitmap_bounds_loader(createImageLoader(true)), _resource_manager(createResourceManager())
{
}

const sp<ResourceManager>& ApplicationResource::resourceManager() const
{
    return _resource_manager;
}

const sp<Dictionary<document>>& ApplicationResource::documents() const
{
    return _documents;
}

const sp<ImageAsset>& ApplicationResource::imageResource() const
{
    return _bitmap_loader;
}

document ApplicationResource::loadDocument(const String& name) const
{
    return _documents->get(name);
}

bitmap ApplicationResource::loadBitmap(const String& name) const
{
    return _bitmap_loader->get(name);
}

bitmap ApplicationResource::loadBitmapBounds(const String& name) const
{
    return _bitmap_bounds_loader->get(name);
}

sp<BitmapLoader> ApplicationResource::getBitmapLoader(const String& name) const
{
    return _bitmap_loader->getLoader(name);
}

sp<ImageAsset> ApplicationResource::createImageLoader(bool justDecodeBounds) const
{
#ifdef ARK_USE_STB_IMAGE
    const sp<ImageAsset> imageResource = sp<ImageAsset>::make(_images, sp<STBBitmapLoader>::make(justDecodeBounds));
#else
    const sp<ImageAsset> imageResource = sp<ImageAsset>::make(_images, nullptr);
#endif
    imageResource->addLoader("png", sp<PNGBitmapLoader>::make(justDecodeBounds));
#ifdef ARK_USE_LIBJPEG_TURBO
    const sp<BitmapLoader> jpegResource = sp<JPEGBitmapLoader>::make(justDecodeBounds);
    imageResource->addLoader("jpg", jpegResource);
    imageResource->addLoader("jpeg", jpegResource);
#endif
    return imageResource;
}

sp<ResourceManager> ApplicationResource::createResourceManager() const
{
    return sp<ResourceManager>::adopt(new ResourceManager(_bitmap_loader, _bitmap_bounds_loader));
}

}
