#pragma once

#include <cmft/cubemapfilter.h>

#include "graphics/forwarding.h"

namespace ark {
namespace plugin {
namespace cmft {

class CMFTUtil {
public:
    static ::cmft::Image bitmapToImage(const Bitmap& bitmap);

};


}
}
}
