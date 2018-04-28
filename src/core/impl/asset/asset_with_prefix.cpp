#include "core/impl/asset/asset_with_prefix.h"

namespace ark {

AssetWithPrefix::AssetWithPrefix(const sp<Asset>& delegate, const String& prefix)
    : _delegate(delegate), _prefix(prefix)
{
    NOT_NULL(_delegate);
}

sp<Readable> AssetWithPrefix::get(const String& name)
{
    return _delegate->get(_prefix + name);
}

sp<Asset> AssetWithPrefix::getAsset(const String& path)
{
    return _delegate->getAsset(_prefix + path);
}

}
