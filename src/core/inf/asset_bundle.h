#ifndef ARK_CORE_INF_ASSET_BUNDLE_H_
#define ARK_CORE_INF_ASSET_BUNDLE_H_

#include "core/forwarding.h"
#include "core/inf/dictionary.h"

namespace ark {

class AssetBundle : public Dictionary<sp<Asset>> {
public:
    virtual ~AssetBundle() = default;

    virtual sp<AssetBundle> getBundle(const String& path) = 0;
};

}

#endif
