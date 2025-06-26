#pragma once

#include "core/inf/loader.h"

#include "graphics/forwarding.h"

namespace ark {

class STBBitmapLoader final : public BitmapLoader {
public:
    STBBitmapLoader(bool justDecodeBounds);

    sp<Bitmap> load(const sp<Readable>& readable) override;

private:
    bool _just_decode_bounds;
};

}
