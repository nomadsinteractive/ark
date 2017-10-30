#ifndef ARK_PLATFORMS_ANDROID_IMPL_RESOURCE_ASSET_RESOURCE_H_
#define ARK_PLATFORMS_ANDROID_IMPL_RESOURCE_ASSET_RESOURCE_H_

#include <android/asset_manager.h>

#include "core/inf/readable.h"
#include "core/inf/dictionary.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

namespace ark {
namespace platform {
namespace android {

class AssetResource : public Asset {
public:
    AssetResource(AAssetManager* assetManager, const String& dirname);

    sp<Readable> get(const String& name) override;

private:
    AAssetManager* _asset_manager;
	String _dirname;
};

}
}
}

#endif
