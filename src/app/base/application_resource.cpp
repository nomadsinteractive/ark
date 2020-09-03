#include "app/base/application_resource.h"

#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/impl/dictionary/loader_bundle.h"
#include "core/impl/loader/document_loader_xml.h"

#include "graphics/impl/bitmap_loader/jpeg_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/png_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/stb_bitmap_loader.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"

namespace ark {

ApplicationResource::ApplicationResource(sp<AssetBundle> assetBundle)
    : _asset_bundle(std::move(assetBundle)), _document_bundle(createDocumentBundle()), _bitmap_bundle(createImageBundle(false)), _bitmap_bounds_loader(createImageBundle(true)), _recycler(sp<Recycler>::make())
{
}

const sp<Recycler>& ApplicationResource::recycler() const
{
    return _recycler;
}

const sp<DocumentBundle>& ApplicationResource::documents() const
{
    return _document_bundle;
}

const sp<BitmapBundle>& ApplicationResource::bitmapBundle() const
{
    return _bitmap_bundle;
}

const sp<BitmapBundle>& ApplicationResource::bitmapBoundsBundle() const
{
    return _bitmap_bounds_loader;
}

document ApplicationResource::loadDocument(const String& name) const
{
    return _document_bundle->get(name);
}

sp<DocumentBundle> ApplicationResource::createDocumentBundle() const
{
    const sp<DocumentBundle> documentBundle = sp<DocumentBundle>::make(_asset_bundle);
    documentBundle->addLoader("xml", sp<DocumentLoaderXML>::make());
    return documentBundle;
}

sp<BitmapBundle> ApplicationResource::createImageBundle(bool justDecodeBounds) const
{
#ifdef ARK_USE_STB_IMAGE
    const sp<BitmapBundle> imageBundle = sp<BitmapBundle>::make(_asset_bundle, sp<STBBitmapLoader>::make(justDecodeBounds));
#else
    const sp<BitmapBundle> imageBundle = sp<BitmapBundle>::make(_asset_bundle, nullptr);
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
