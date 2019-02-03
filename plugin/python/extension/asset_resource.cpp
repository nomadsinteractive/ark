#include "python/extension/asset_resource.h"

#include "core/inf/asset.h"
#include "core/util/strings.h"

namespace ark {
namespace plugin {
namespace python {

AssetResource::AssetResource(const sp<Asset>& resource)
    : _asset(resource)
{
}

sp<String> AssetResource::getString(const String& filepath)
{
    const sp<Readable> readable = _asset->get(filepath);
    return readable ? sp<String>::make(Strings::loadFromReadable(readable)) : nullptr;
}

String AssetResource::getRealPath(const String& filepath)
{
    return _asset->getRealPath(filepath);
}

}
}
}
