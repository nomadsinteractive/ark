#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"

namespace ark {

AssetBundleWithPrefix::AssetBundleWithPrefix(const sp<AssetBundle>& delegate, const String& prefix)
    : _delegate(delegate), _prefix(prefix)
{
    DASSERT(_delegate);
}

sp<Asset> AssetBundleWithPrefix::getAsset(const String& name)
{
    return _delegate->getAsset(_prefix + name);
}

sp<AssetBundle> AssetBundleWithPrefix::getBundle(const String& path)
{
    return _delegate->getBundle(_prefix + path);
}

}
