#ifndef ARK_PLATFORM_DARWIN_IMPL_ASSET_BUNDLE_DARWIN_H_
#define ARK_PLATFORM_DARWIN_IMPL_ASSET_BUNDLE_DARWIN_H_

#include <CoreFoundation/CFString.h>

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset_bundle.h"

namespace ark {
namespace platform {
namespace darwin {

class AssetBundleDarwin : public AssetBundle {
public:
    AssetBundleDarwin(const String& directory);

    virtual sp<Asset> get(const String& name) override;
    virtual sp<AssetBundle> getBundle(const String& path) override;

    static bool exists(const String& location);

private:
    String _directory;
};

}
}
}

#endif
