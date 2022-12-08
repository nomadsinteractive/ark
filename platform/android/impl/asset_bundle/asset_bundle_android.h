#ifndef ARK_PLATFORMS_ANDROID_IMPL_RESOURCE_ASSET_RESOURCE_H_
#define ARK_PLATFORMS_ANDROID_IMPL_RESOURCE_ASSET_RESOURCE_H_

#include <android/asset_manager.h>

#include "core/inf/asset_bundle.h"
#include "core/inf/readable.h"
#include "core/base/string.h"

namespace ark {
namespace platform {
namespace android {

class AssetBundleAndroid : public AssetBundle {
public:
    AssetBundleAndroid(AAssetManager* assetManager, const String& dirname);

    virtual sp<Asset> getAsset(const String& name) override;
    virtual sp<AssetBundle> getBundle(const String& path) override;

private:
    AAssetManager* _asset_manager;
	String _dirname;
};

}
}
}

#endif
