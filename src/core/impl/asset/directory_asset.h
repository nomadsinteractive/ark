#ifndef ARK_IMPL_ASSET_DIRECTORY_ASSET_H_
#define ARK_IMPL_ASSET_DIRECTORY_ASSET_H_

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "core/inf/dictionary.h"

namespace ark {

class DirectoryAsset : public Asset {
public:
    DirectoryAsset(const String& directory);

    virtual sp<Readable> get(const String& name) override;

private:
    String _directory;
};

}

#endif
