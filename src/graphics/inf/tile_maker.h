#ifndef ARK_GRAPHICS_INF_TILE_MAKER_H_
#define ARK_GRAPHICS_INF_TILE_MAKER_H_

#include "graphics/forwarding.h"

namespace ark {

class TileMaker {
public:
    virtual ~TileMaker() = default;

    virtual sp<Renderer> makeTile(int32_t x, int32_t y) = 0;
};

}

#endif
