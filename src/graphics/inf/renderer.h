#ifndef ARK_GRAPHICS_INF_RENDERER_H_
#define ARK_GRAPHICS_INF_RENDERER_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Renderer {
public:
    virtual ~Renderer() = default;

    virtual void render(RenderRequest& renderRequest, float x, float y) = 0;

    class ARK_API Group {
    public:
        virtual ~Group() = default;

        virtual void addRenderer(const sp<Renderer>& renderer) = 0;
    };

};

}

#endif
