#include "python/extension/asset_resource.h"

#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
#include "core/util/strings.h"

namespace ark {
namespace plugin {
namespace python {

AssetResource::AssetResource(const sp<AssetBundle>& resource)
    : _asset(resource)
{
}

sp<String> AssetResource::getString(const String& filepath)
{
    const sp<Asset> asset = _asset->get(filepath);
    const sp<Readable> readable = asset ? asset->open() : nullptr;
    return readable ? sp<String>::make(Strings::loadFromReadable(readable)) : nullptr;
}

String AssetResource::getRealPath(const String& filepath)
{
    const sp<Asset> asset = _asset->get(filepath);
    return asset ? asset->location() : filepath;
}

}
}
}
