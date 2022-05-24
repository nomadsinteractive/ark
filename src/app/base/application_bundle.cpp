#include "app/base/application_bundle.h"

#include "core/base/string.h"
#include "core/inf/dictionary.h"
#include "core/impl/dictionary/loader_bundle.h"
#include "core/impl/loader/document_loader_xml.h"
#include "core/impl/loader/json_loader_text.h"
#include "core/impl/loader/string_loader_text.h"

#include "graphics/impl/bitmap_loader/jpeg_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/png_bitmap_loader.h"
#include "graphics/impl/bitmap_loader/stb_bitmap_loader.h"

#include "renderer/base/recycler.h"
#include "renderer/base/render_controller.h"

namespace ark {

ApplicationBundle::ApplicationBundle(sp<AssetBundle> assetBundle)
    : _asset_bundle(std::move(assetBundle)), _document_loader_bundle(createDocumentLoaderBundle()), _json_loader_bundle(createJsonLoaderBundle()), _string_loader_bundle(createStringLoaderBundle()),
      _bitmap_bundle(createImageLoaderBundle(false)), _bitmap_bounds_loader(createImageLoaderBundle(true)), _recycler(sp<Recycler>::make())
{
}

const sp<Recycler>& ApplicationBundle::recycler() const
{
    return _recycler;
}

const sp<DocumentLoaderBundle>& ApplicationBundle::documents() const
{
    return _document_loader_bundle;
}

const sp<BitmapLoaderBundle>& ApplicationBundle::bitmapBundle() const
{
    return _bitmap_bundle;
}

const sp<BitmapLoaderBundle>& ApplicationBundle::bitmapBoundsBundle() const
{
    return _bitmap_bounds_loader;
}

document ApplicationBundle::loadDocument(const String& resid) const
{
    return _document_loader_bundle->get(resid);
}

Json ApplicationBundle::loadJson(const String& resid) const
{
    return _json_loader_bundle->get(resid);
}

String ApplicationBundle::loadString(const String& resid) const
{
    return _string_loader_bundle->get(resid);
}

sp<DocumentLoaderBundle> ApplicationBundle::createDocumentLoaderBundle() const
{
    const sp<DocumentLoaderBundle> documentBundle = sp<DocumentLoaderBundle>::make(_asset_bundle);
    documentBundle->addLoader("xml", sp<DocumentLoaderXML>::make());
    return documentBundle;
}

sp<JsonLoaderBundle> ApplicationBundle::createJsonLoaderBundle() const
{
    const sp<JsonLoaderBundle> documentBundle = sp<JsonLoaderBundle>::make(_asset_bundle);
    documentBundle->addLoader("json", sp<JsonLoaderText>::make());
    return documentBundle;
}

sp<StringLoaderBundle> ApplicationBundle::createStringLoaderBundle() const
{
    return sp<StringLoaderBundle>::make(_asset_bundle, sp<StringLoaderText>::make());
}

sp<BitmapLoaderBundle> ApplicationBundle::createImageLoaderBundle(bool justDecodeBounds) const
{
#ifdef ARK_USE_STB_IMAGE
    const sp<BitmapLoaderBundle> imageBundle = sp<BitmapLoaderBundle>::make(_asset_bundle, sp<STBBitmapLoader>::make(justDecodeBounds));
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
