#ifndef ARK_IMPL_ASSET_BUNDLE_WITH_PREFIX_H_
#define ARK_IMPL_ASSET_BUNDLE_WITH_PREFIX_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetBundleWithPrefix : public AssetBundle {
public:
    AssetBundleWithPrefix(sp<AssetBundle> delegate, String prefix);

    virtual sp<Asset> getAsset(const String& name) override;
    virtual sp<AssetBundle> getBundle(const String& path) override;

private:
    sp<AssetBundle> _delegate;
    String _prefix;

};

}

#endif
