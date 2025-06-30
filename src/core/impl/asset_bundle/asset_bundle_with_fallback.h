#pragma once

#include "core/forwarding.h"
#include "core/inf/asset_bundle.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetBundleWithFallback final : public AssetBundle {
public:
    AssetBundleWithFallback(sp<AssetBundle> delegate, sp<AssetBundle> fallback);

    sp<Asset> getAsset(const String& name) override;
    sp<AssetBundle> getBundle(const String& path) override;
    Vector<sp<Asset>> listAssets(const String& regex) override;

private:
    sp<AssetBundle> _delegate;
    sp<AssetBundle> _fallback;
};

}
