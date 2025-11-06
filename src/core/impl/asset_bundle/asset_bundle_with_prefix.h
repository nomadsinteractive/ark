#pragma once

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetBundleWithPrefix final : public AssetBundle {
public:
    AssetBundleWithPrefix(sp<AssetBundle> delegate, String prefix);

    sp<Asset> getAsset(const String& name) override;
    sp<AssetBundle> getBundle(const String& path) override;

    Vector<String> listAssets(StringView dirname) override;

private:
    sp<AssetBundle> _delegate;
    String _prefix;

};

}
