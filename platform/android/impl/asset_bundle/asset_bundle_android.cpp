#include "platform/android/impl/asset_bundle/asset_bundle_android.h"

#include "core/inf/asset.h"
#include "core/types/shared_ptr.h"

#include "platform/android/impl/readable/asset_readable.h"

namespace ark {
namespace platform {
namespace android {

namespace {

class AndroidAsset : public Asset {
public:
    AndroidAsset(AAssetManager* assetManager, const String& filepath)
        : _asset_manager(assetManager), _filepath(filepath) {
    }

    virtual sp<Readable> open() override {
        AAsset* asset = AAssetManager_open(_asset_manager, _filepath.c_str(), AASSET_MODE_UNKNOWN);
        return asset ? sp<Readable>::adopt(new AssetReadable(asset)) : nullptr;
    }

    virtual String location() override {
        return _filepath;
    }

private:
    AAssetManager* _asset_manager;
    String _filepath;
};

}

AssetBundleAndroid::AssetBundleAndroid(AAssetManager *assetManager, const String& dirname)
    : _asset_manager(assetManager), _dirname(dirname)
{
}

sp<Asset> AssetBundleAndroid::get(const String& name)
{
    const String filepath = _dirname == "." ? name : _dirname + "/" + name;
    return sp<AndroidAsset>::make(_asset_manager, filepath);
}

sp<AssetBundle> AssetBundleAndroid::getBundle(const String& path)
{
    return sp<AssetBundleAndroid>::make(_asset_manager, _dirname + path);
}

}
}
}
