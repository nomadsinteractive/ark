#ifndef ARK_GRAPHICS_IMPL_RESOURCE_STB_IMAGE_RESOURCE_H_
#define ARK_GRAPHICS_IMPL_RESOURCE_STB_IMAGE_RESOURCE_H_

#include "core/inf/dictionary.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class STBImageResource : public Dictionary<bitmap> {
public:
    STBImageResource(const sp<Asset>& asset, bool justDecodeBounds = false);

    virtual bitmap get(const String& name) override;

private:
    bitmap load(const sp<Readable>& readable);

private:
    sp<Asset> _asset;
    bool _just_decode_bounds;
};

}

#endif
