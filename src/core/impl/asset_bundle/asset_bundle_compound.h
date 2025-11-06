#pragma once

#include "core/forwarding.h"
#include "core/inf/asset_bundle.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetBundleCompound final : public AssetBundle {
public:
    AssetBundleCompound(sp<AssetBundle> a1, sp<AssetBundle> a2);

    sp<Asset> getAsset(const String& name) override;
    sp<AssetBundle> getBundle(const String& path) override;

    Vector<String> listAssets(StringView dirname) override;

private:
    sp<AssetBundle> _a1;
    sp<AssetBundle> _a2;
};

}
