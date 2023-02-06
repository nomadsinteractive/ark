#ifndef ARK_IMPL_ASSET_BUNDLE_DIRECTORY_H_
#define ARK_IMPL_ASSET_BUNDLE_DIRECTORY_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"

namespace ark {

class AssetBundleDirectory : public AssetBundle {
public:
    AssetBundleDirectory(String directory);

    virtual sp<Asset> getAsset(const String& name) override;
    virtual sp<AssetBundle> getBundle(const String& path) override;
    virtual std::vector<sp<Asset>> listAssets(const String& regex) override;

private:
    String _directory;

};

}

#endif
