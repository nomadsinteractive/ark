#pragma once

#include "core/base/api.h"
#include "core/base/json.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API ApplicationBundle {
public:
    ApplicationBundle(sp<AssetBundle> assetBundle);

    const sp<DocumentLoaderBundle>& documents() const;

    const sp<BitmapLoaderBundle>& bitmapBundle() const;
    const sp<BitmapLoaderBundle>& bitmapBoundsBundle() const;

//  [[script::bindings::auto]]
    document loadDocument(const String& resid) const;
//  [[script::bindings::auto]]
    Json loadJson(const String& resid) const;
//  [[script::bindings::auto]]
    String loadString(const String& resid) const;

private:
    sp<DocumentLoaderBundle> createDocumentLoaderBundle() const;
    sp<JsonLoaderBundle> createJsonLoaderBundle() const;
    sp<StringLoaderBundle> createStringLoaderBundle() const;

    sp<BitmapLoaderBundle> createImageLoaderBundle(bool justDecodeBounds) const;

private:
    sp<AssetBundle> _asset_bundle;
    sp<AssetBundle> _fonts;

    sp<DocumentLoaderBundle> _document_loader_bundle;
    sp<JsonLoaderBundle> _json_loader_bundle;
    sp<StringLoaderBundle> _string_loader_bundle;

    sp<BitmapLoaderBundle> _bitmap_bundle;
    sp<BitmapLoaderBundle> _bitmap_bounds_loader;
};

}
