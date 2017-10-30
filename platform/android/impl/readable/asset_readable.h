#ifndef ARK_PLATFORMS_ANDROID_IMPL_READABLE_ASSET_READABLE_H_
#define ARK_PLATFORMS_ANDROID_IMPL_READABLE_ASSET_READABLE_H_

#include <android/asset_manager.h>

#include "core/inf/readable.h"

namespace ark {
namespace platform {
namespace android {

class AssetReadable : public Readable {
public:
    AssetReadable(AAsset* asset);
    ~AssetReadable();

    virtual uint32_t read(void *buffer, uint32_t bufferSize) override;
	virtual int32_t seek(int32_t position, int32_t whence) override;
	virtual int32_t remaining() override;

private:
    AAsset* _asset;
};

}
}
}

#endif
