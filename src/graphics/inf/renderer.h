#ifndef ARK_GRAPHICS_INF_RENDERER_H_
#define ARK_GRAPHICS_INF_RENDERER_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Renderer {
public:
    virtual ~Renderer() = default;

    virtual void render(RenderRequest& renderRequest, float x, float y) = 0;

//  [[script::bindings::meta(addRenderer())]]
//  [[script::bindings::meta(expire())]]
//  [[script::bindings::meta(isExpired())]]
//  [[script::bindings::meta(size())]]

    class ARK_API Group {
    public:
        virtual ~Group() = default;

        virtual void addRenderer(const sp<Renderer>& drawable) = 0;
    };

};

}

#endif
