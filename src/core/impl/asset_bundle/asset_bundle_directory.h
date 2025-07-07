#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"

namespace ark {

class AssetBundleDirectory final : public AssetBundle {
public:
    AssetBundleDirectory(String directory);

    sp<Asset> getAsset(const String& name) override;
    sp<AssetBundle> getBundle(const String& path) override;
    Vector<sp<Asset>> listAssets(const String& regex) override;

private:
    String _directory;
};

}
