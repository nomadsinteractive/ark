#ifndef ARK_CORE_IMPL_ASSET_ASSET_FILE_H_
#define ARK_CORE_IMPL_ASSET_ASSET_FILE_H_

#include "core/base/string.h"
#include "core/inf/asset.h"

namespace ark {

class AssetFile : public Asset {
public:
    AssetFile(const String& filepath);

    virtual sp<Readable> open() override;
    virtual String location() override;

private:
    String _filepath;

};

}

#endif
