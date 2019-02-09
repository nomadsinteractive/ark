#include "core/impl/dictionary/xml_directory.h"

#include "core/inf/asset.h"
#include "core/inf/asset_bundle.h"
#include "core/types/null.h"
#include "core/util/documents.h"

namespace ark {

XMLDirectory::XMLDirectory(const sp<AssetBundle>& resource)
    : _resource(resource) {
    DASSERT(_resource);
}

document XMLDirectory::get(const String& name)
{
    const sp<Asset> asset = _resource->get(name);
    return asset ? Documents::loadFromReadable(asset->open()) : document::null();
}

}
