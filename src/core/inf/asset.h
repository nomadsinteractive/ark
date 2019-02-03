#ifndef ARK_CORE_INF_ASSET_H_
#define ARK_CORE_INF_ASSET_H_

#include "core/forwarding.h"
#include "core/inf/dictionary.h"

namespace ark {

class Asset : public Dictionary<sp<Readable>> {
public:
    virtual ~Asset() = default;

    virtual sp<Asset> getAsset(const String& path) = 0;
    virtual String getRealPath(const String& path) = 0;
};

}

#endif
