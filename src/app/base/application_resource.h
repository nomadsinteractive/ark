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
    ApplicationResource(const sp<Dictionary<document>>& documents, const sp<Asset>& images);

    const sp<ResourceManager>& resourceManager() const;

    const sp<Dictionary<document>>& documents() const;
    const sp<ImageBundle>& imageResource() const;

    document loadDocument(const String& name) const;

    bitmap loadBitmap(const String& name) const;
    bitmap loadBitmapBounds(const String& name) const;

//    sp<BitmapLoader> getBitmapLoader(const String& name) const;

private:
    sp<ImageBundle> createImageLoader(bool justDecodeBounds) const;
    sp<ResourceManager> createResourceManager() const;

private:
    sp<Asset> _images;
    sp<Asset> _fonts;
    sp<Dictionary<document>> _documents;

    sp<ImageBundle> _bitmap_loader;
    sp<Dictionary<bitmap>> _bitmap_bounds_loader;

    sp<ResourceManager> _resource_manager;
};

}

#endif
