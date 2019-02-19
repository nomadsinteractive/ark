#ifndef ARK_PLATFORMS_ANDROID_IMPL_PLATFORM_ANDROID_H_
#define ARK_PLATFORMS_ANDROID_IMPL_PLATFORM_ANDROID_H_

#include <android/asset_manager.h>

#include <android_native_app_glue.h>

#include "core/forwarding.h"


namespace ark {
namespace platform {
namespace android {

class PlatformAndroid {
public:
	PlatformAndroid(android_app* state);
	
	android_app* state();
	
	sp<AssetBundle> getAssetBundle(const String& path, const String& appPath);
	String getUserStoragePath(const String& filename);
	
private:
	android_app* _state;
	AAssetManager* _asset_manager;
};

}
}
}

#endif
