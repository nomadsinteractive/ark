#ifndef ARK_RENDERER_BASE_BITMAP_BUNDLE_H_
#define ARK_RENDERER_BASE_BITMAP_BUNDLE_H_

#include <unordered_map>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API BitmapBundle {
public:
    BitmapBundle(const Atlas& atlas, sp<Bitmap> atlasBitmap);

//  [[script::bindings::auto]]
    sp<Bitmap> getBitmap(int32_t resid);

private:
    const Atlas& _atlas;
    sp<Bitmap> _atlas_bitmap;
    std::unordered_map<int32_t, sp<Bitmap>> _bitmaps;
};

}

#endif
