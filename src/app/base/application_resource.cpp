#include "app/base/application_resource.h"

#include "core/base/string.h"
#include "core/inf/dictionary.h"

#include "graphics/base/bitmap_bundle.h"
#include "graphics/impl/bitmap_loader/jpeg_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/png_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/stb_bitmap_loader.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"

namespace ark {

ApplicationResource::ApplicationResource(const sp<Dictionary<document>>& documents, const sp<AssetBundle>& images)
    : _images(images), _documents(documents), _bitmap_bundle(createImageLoader(false)), _bitmap_bounds_loader(createImageLoader(true)), _recycler(sp<Recycler>::make())
{
}

const sp<Recycler>& ApplicationResource::recycler() const
{
    return _recycler;
}

const sp<Dictionary<document>>& ApplicationResource::documents() const
{
    return _documents;
}

const sp<BitmapBundle>& ApplicationResource::bitmapBundle() const
{
    return _bitmap_bundle;
}

const sp<Dictionary<bitmap>>& ApplicationResource::bitmapBoundsLoader() const
{
    return _bitmap_bounds_loader;
}

document ApplicationResource::loadDocument(const String& name) const
{
    return _documents->get(name);
}

sp<BitmapBundle> ApplicationResource::createImageLoader(bool justDecodeBounds) const
{
#ifdef ARK_USE_STB_IMAGE
    const sp<BitmapBundle> imageBundle = sp<BitmapBundle>::make(_images, sp<STBBitmapLoader>::make(justDecodeBounds));
#else
    const sp<BitmapBundle> imageBundle = sp<BitmapBundle>::make(_images, nullptr);
#endif
    imageBundle->addLoader("png", sp<PNGBitmapLoader>::make(justDecodeBounds));
#ifdef ARK_USE_LIBJPEG_TURBO
    const sp<BitmapLoader> jpegLoader = sp<JPEGBitmapLoader>::make(justDecodeBounds);
    imageBundle->addLoader("jpg", jpegLoader);
    imageBundle->addLoader("jpeg", jpegLoader);
#endif
    return imageBundle;
}

}
