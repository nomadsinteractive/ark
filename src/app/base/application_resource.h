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
    ApplicationResource(const sp<Dictionary<document>>& documents, const sp<AssetBundle>& images);

    const sp<Recycler>& recycler() const;

    const sp<Dictionary<document>>& documents() const;
    const sp<BitmapBundle>& bitmapBundle() const;
    const sp<Dictionary<bitmap>>& bitmapBoundsLoader() const;

    document loadDocument(const String& name) const;

private:
    sp<BitmapBundle> createImageLoader(bool justDecodeBounds) const;

private:
    sp<AssetBundle> _images;
    sp<AssetBundle> _fonts;
    sp<Dictionary<document>> _documents;

    sp<BitmapBundle> _bitmap_bundle;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    sp<Recycler> _recycler;
};

}

#endif
