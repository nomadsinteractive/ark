#ifndef ARK_GRAPHICS_UTIL_RENDERER_UTIL_H_
#define ARK_GRAPHICS_UTIL_RENDERER_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Renderer")]]
class ARK_API RendererUtil final {
public:

//[[script::bindings::classmethod]]
    static void addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer);

//[[script::bindings::classmethod]]
    static void dispose(const sp<Renderer>& self);

//[[script::bindings::property]]
    static SafePtr<Size> size(const sp<Renderer>& self);

//[[script::bindings::classmethod]]
    static sp<Renderer> translate(const sp<Renderer>& self, const sp<Vec2>& position);

};

}

#endif
