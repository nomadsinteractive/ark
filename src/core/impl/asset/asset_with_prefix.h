#ifndef ARK_IMPL_ASSET_ASSET_WITH_PREFIX_H_
#define ARK_IMPL_ASSET_ASSET_WITH_PREFIX_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetWithPrefix : public Asset {
public:
    AssetWithPrefix(const sp<Asset>& delegate, const String& prefix);

    virtual sp<Readable> get(const String& name) override;
    virtual sp<Asset> getAsset(const String& path) override;

private:
    sp<Asset> _delegate;
    String _prefix;
};

}

#endif
