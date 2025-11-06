#include "core/impl/asset_bundle/asset_bundle_compound.h"

#include "core/base/string.h"

namespace ark {

AssetBundleCompound::AssetBundleCompound(sp<AssetBundle> a1, sp<AssetBundle> a2)
    : _a1(std::move(a1)), _a2(std::move(a2))
{
    ASSERT(_a1 && _a2);
}

sp<Asset> AssetBundleCompound::getAsset(const String& name)
{
    sp<Asset> asset = _a1->getAsset(name);
    return asset ? std::move(asset) : _a2->getAsset(name);
}

sp<AssetBundle> AssetBundleCompound::getBundle(const String& path)
{
    sp<AssetBundle> ab1 = _a1->getBundle(path);
    sp<AssetBundle> ab2 = _a2->getBundle(path);
    if(ab1 && ab2)
        return sp<AssetBundle>::make<AssetBundleCompound>(std::move(ab1), std::move(ab2));
    return ab1 ? std::move(ab1) : std::move(ab2);
}

Vector<String> AssetBundleCompound::listAssets(const StringView dirname)
{
    Vector<String> assets = _a1->listAssets(dirname);
    for(String& i : _a2->listAssets(dirname))
        assets.push_back(std::move(i));
    return assets;
}

}
