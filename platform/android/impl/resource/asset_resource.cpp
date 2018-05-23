#include "platform/android/impl/resource/asset_resource.h"

#include "core/types/shared_ptr.h"

#include "platform/android/impl/readable/asset_readable.h"

namespace ark {
namespace platform {
namespace android {

AssetResource::AssetResource(AAssetManager *assetManager, const String& dirname)
    : _asset_manager(assetManager), _dirname(dirname)
{
}

sp<Readable> AssetResource::get(const String& name)
{
	const String path = _dirname == "." ? name : _dirname + "/" + name;
    AAsset* asset = AAssetManager_open(_asset_manager, path.c_str(), AASSET_MODE_UNKNOWN);
    return asset ? sp<Readable>::adopt(new AssetReadable(asset)) : sp<Readable>::null();
}

sp<Asset> AssetResource::getAsset(const String& path)
{
    return sp<AssetResource>::make(_asset_manager, _dirname + path);
}

}
}
}
