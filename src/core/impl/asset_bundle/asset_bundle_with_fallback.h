#ifndef ARK_IMPL_ASSET_BUNDLE_WITH_FALLBACK_H_
#define ARK_IMPL_ASSET_BUNDLE_WITH_FALLBACK_H_

#include "core/forwarding.h"
#include "core/inf/asset_bundle.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetBundleWithFallback : public AssetBundle {
public:
    AssetBundleWithFallback(sp<AssetBundle> delegate, sp<AssetBundle> fallback);

    virtual sp<Asset> getAsset(const String& name) override;
    virtual sp<AssetBundle> getBundle(const String& path) override;
    virtual std::vector<sp<Asset>> listAssets(const String& regex) override;

private:
    sp<AssetBundle> _delegate;
    sp<AssetBundle> _fallback;

};

}

#endif
