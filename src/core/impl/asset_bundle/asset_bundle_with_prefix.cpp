#include "core/impl/asset_bundle/asset_bundle_with_prefix.h"

namespace ark {

AssetBundleWithPrefix::AssetBundleWithPrefix(sp<AssetBundle> delegate, String prefix)
    : _delegate(std::move(delegate)), _prefix(std::move(prefix))
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
