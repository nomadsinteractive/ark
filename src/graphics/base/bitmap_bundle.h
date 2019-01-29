#ifndef ARK_GRAPHICS_BASE_BITMAP_BUNDLE_H_
#define ARK_GRAPHICS_BASE_BITMAP_BUNDLE_H_

#include <map>

#include "core/base/api.h"
#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API BitmapBundle : public Dictionary<bitmap> {
public:
    BitmapBundle(const sp<Asset>& asset, const sp<BitmapLoader>& defaultLoader);

    virtual bitmap get(const String& name) override;

    void addLoader(const String& ext, const sp<BitmapLoader>& loader);
    const sp<BitmapLoader>& getLoader(const String& ext) const;

private:
    sp<Asset> _asset;

    std::map<String, sp<BitmapLoader>> _loaders;
    sp<BitmapLoader> _default_loader;

};

}

#endif
