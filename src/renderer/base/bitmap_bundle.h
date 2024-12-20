#pragma once

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
    sp<Bitmap> getBitmap(const NamedHash& resid) const;

private:
    const Atlas& _atlas;
    sp<Bitmap> _atlas_bitmap;
};

}
