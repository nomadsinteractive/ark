#ifndef ARK_CORE_INF_ASSET_BUNDLE_H_
#define ARK_CORE_INF_ASSET_BUNDLE_H_

#include <vector>

#include "core/forwarding.h"

namespace ark {

class AssetBundle {
public:
    virtual ~AssetBundle() = default;

    virtual sp<Asset> getAsset(const String& path) = 0;
    virtual sp<AssetBundle> getBundle(const String& path) = 0;

    virtual std::vector<sp<Asset>> listAssets(const String& regex) = 0;
};

}

#endif
