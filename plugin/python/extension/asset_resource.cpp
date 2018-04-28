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

String AssetResource::getString(const String& filepath)
{
    const sp<Readable> readable = _asset->get(filepath);
    return readable ? Strings::loadFromReadable(readable) : "";
}

}
}
}
