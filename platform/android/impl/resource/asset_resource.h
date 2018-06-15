#ifndef ARK_PLATFORMS_ANDROID_IMPL_RESOURCE_ASSET_RESOURCE_H_
#define ARK_PLATFORMS_ANDROID_IMPL_RESOURCE_ASSET_RESOURCE_H_

#include <android/asset_manager.h>

#include "core/inf/asset.h"
#include "core/inf/readable.h"
#include "core/base/string.h"

namespace ark {
namespace platform {
namespace android {

class AssetResource : public Asset {
public:
    AssetResource(AAssetManager* assetManager, const String& dirname);

    virtual sp<Readable> get(const String& name) override;
    virtual sp<Asset> getAsset(const String& path) override;

private:
    AAssetManager* _asset_manager;
	String _dirname;
};

}
}
}

#endif
