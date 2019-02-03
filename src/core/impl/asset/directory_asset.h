#ifndef ARK_IMPL_ASSET_DIRECTORY_ASSET_H_
#define ARK_IMPL_ASSET_DIRECTORY_ASSET_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/inf/asset.h"

namespace ark {

class DirectoryAsset : public Asset {
public:
    DirectoryAsset(const String& directory);

    virtual sp<Readable> get(const String& name) override;
    virtual sp<Asset> getAsset(const String& path) override;
    virtual String getRealPath(const String& path) override;

private:
    String _directory;

};

}

#endif
