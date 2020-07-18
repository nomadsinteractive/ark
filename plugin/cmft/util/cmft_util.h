#ifndef ARK_PLUGIN_CMFT_UTIL_CMFT_UTIL_H_
#define ARK_PLUGIN_CMFT_UTIL_CMFT_UTIL_H_

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

#endif
