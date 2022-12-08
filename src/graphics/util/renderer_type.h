#ifndef ARK_GRAPHICS_UTIL_RENDERER_TYPE_H_
#define ARK_GRAPHICS_UTIL_RENDERER_TYPE_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Renderer")]]
class ARK_API RendererType final {
public:
//[[script::bindings::constructor]]
    static sp<Renderer> create(const sp<Renderer>& delegate = nullptr);

//[[script::bindings::classmethod]]
    static void addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer);
//[[script::bindings::classmethod]]
    static sp<Renderer> wrap(const sp<Renderer>& self);

//[[script::bindings::classmethod]]
    static sp<Renderer> makeDisposable(const sp<Renderer>& self, const sp<Boolean>& disposed = nullptr);
//[[script::bindings::classmethod]]
    static sp<Renderer> makeVisible(const sp<Renderer>& self, const sp<Boolean>& visibility);
//[[script::bindings::classmethod]]
    static sp<Renderer> makeAutoRelease(const sp<Renderer>& self, int32_t refCount = 1);

//[[script::bindings::property]]
    static SafePtr<Size> size(const sp<Renderer>& self);
//[[script::bindings::property]]
    static const sp<Renderer>& wrapped(const sp<Renderer>& self);
//[[script::bindings::property]]
    static void setWrapped(const sp<Renderer>& self, sp<Renderer> wrapped);

//[[script::bindings::classmethod]]
    static sp<Renderer> translate(const sp<Renderer>& self, const sp<Vec3>& position);

};

}

#endif
