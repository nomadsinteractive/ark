#ifndef ARK_GRAPHICS_INF_RENDERER_MAKER_H_
#define ARK_GRAPHICS_INF_RENDERER_MAKER_H_

#include "graphics/forwarding.h"

namespace ark {

class RendererMaker {
public:
    virtual ~RendererMaker() = default;

    virtual sp<Renderer> make(int32_t x, int32_t y) = 0;
    virtual void recycle(const sp<Renderer>& renderer) {}
};

}

#endif
