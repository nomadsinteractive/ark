#include "core/impl/asset/asset_with_fallback.h"

namespace ark {

AssetWithFallback::AssetWithFallback(const sp<Asset>& delegate, const sp<Asset>& fallback)
    : _delegate(delegate), _fallback(fallback)
{
    NOT_NULL(_delegate);
    NOT_NULL(_fallback);
}

sp<Readable> AssetWithFallback::get(const String& name)
{
    const sp<Readable> readable = _delegate->get(name);
    return readable ? readable : _fallback->get(name);
}

sp<Asset> AssetWithFallback::getAsset(const String& path)
{
    const sp<Asset> asset = _delegate->getAsset(path);
    return asset ? asset : _fallback->getAsset(path);
}

}
