#pragma once

#include "core/inf/loader.h"

#include "graphics/forwarding.h"

namespace ark {

class PNGBitmapLoader final : public BitmapLoader {
public:
    PNGBitmapLoader(bool justDecodeBounds);

    sp<Bitmap> load(const sp<Readable>& readable) override;

private:
    bool _just_decode_bounds;
};

}
