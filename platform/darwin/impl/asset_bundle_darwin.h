#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"

namespace ark {
namespace platform {
namespace darwin {

class AssetBundleDarwin : public AssetBundle {
public:
    AssetBundleDarwin(String directory);

    virtual sp<Asset> getAsset(const String& name) override;
    virtual sp<AssetBundle> getBundle(const String& path) override;
    virtual std::vector<sp<Asset>> listAssets(const String& regex) override;

    static bool exists(const String& location);

private:
    String _directory;
};

}
}
}
