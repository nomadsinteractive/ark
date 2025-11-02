#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"

namespace ark::platform::darwin {

class AssetBundleDarwin : public AssetBundle {
public:
    AssetBundleDarwin(String directory);

    sp<Asset> getAsset(const String& name) override;
    sp<AssetBundle> getBundle(const String& path) override;

    Vector<String> listAssets() override;

private:
    String _directory;
};

}
