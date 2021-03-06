#ifndef ARK_GRAPHICS_IMPL_BITMAP_LOADER_PNG_BITMAP_LOADER_H_
#define ARK_GRAPHICS_IMPL_BITMAP_LOADER_PNG_BITMAP_LOADER_H_

#include "core/inf/loader.h"

#include "graphics/forwarding.h"

namespace ark {

class PNGBitmapLoader : public BitmapLoader {
public:
    PNGBitmapLoader(bool justDecodeBounds);

    virtual sp<Bitmap> load(const sp<Readable>& readable) override;

private:
    bool _just_decode_bounds;
};

}

#endif
