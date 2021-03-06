#include "core/impl/asset_bundle/asset_bundle_with_fallback.h"

namespace ark {

AssetBundleWithFallback::AssetBundleWithFallback(const sp<AssetBundle>& delegate, const sp<AssetBundle>& fallback)
    : _delegate(delegate), _fallback(fallback)
{
    DASSERT(_delegate);
    DASSERT(_fallback);
}

sp<Asset> AssetBundleWithFallback::get(const String& name)
{
    const sp<Asset> asset = _delegate->get(name);
    return asset ? asset : _fallback->get(name);
}

sp<AssetBundle> AssetBundleWithFallback::getBundle(const String& path)
{
    const sp<AssetBundle> assetBundle = _delegate->getBundle(path);
    return assetBundle ? assetBundle : _fallback->getBundle(path);
}

}
