#ifndef ARK_APP_BASE_APPLICATION_RESOURCE_H_
#define ARK_APP_BASE_APPLICATION_RESOURCE_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API ApplicationResource {
public:
    ApplicationResource(sp<AssetBundle> assetBundle);

    const sp<Recycler>& recycler() const;

    const sp<DocumentBundle>& documents() const;

    const sp<BitmapBundle>& bitmapBundle() const;
    const sp<BitmapBundle>& bitmapBoundsBundle() const;

    document loadDocument(const String& name) const;

private:
    sp<DocumentBundle> createDocumentBundle() const;
    sp<BitmapBundle> createImageBundle(bool justDecodeBounds) const;

private:
    sp<AssetBundle> _asset_bundle;
    sp<AssetBundle> _fonts;

    sp<DocumentBundle> _document_bundle;

    sp<BitmapBundle> _bitmap_bundle;
    sp<BitmapBundle> _bitmap_bounds_loader;

    sp<Recycler> _recycler;
};

}

#endif
