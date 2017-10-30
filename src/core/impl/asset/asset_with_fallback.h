#ifndef ARK_IMPL_ASSET_ASSET_WITH_FALLBACK_H_
#define ARK_IMPL_ASSET_ASSET_WITH_FALLBACK_H_

#include "core/forwarding.h"
#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

namespace ark {

class AssetWithFallback : public Asset {
public:
    AssetWithFallback(const sp<Asset>& delegate, const sp<Asset>& fallback);

    virtual sp<Readable> get(const String& name) override;

private:
    sp<Asset> _delegate;
    sp<Asset> _fallback;
};

}

#endif
