#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Renderer")]]
class ARK_API RendererType final {
public:
//  [[script::bindings::enumeration]]
    enum Phrase {
        PHRASE_DEFAULT,
        PHRASE_WIDGET,
        PHRASE_LAYER,
        PHRASE_RENDER_LAYER,
        PHRASE_COUNT
    };

public:
//[[script::bindings::constructor]]
    static sp<Renderer> create(const sp<Renderer>& delegate = nullptr);

//[[script::bindings::classmethod]]
    static void addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer, Traits traits);
//[[script::bindings::classmethod]]
    static sp<Renderer> wrap(const sp<Renderer>& self);
[[deprecated]]
//[[script::bindings::classmethod]]
    static sp<Renderer> makeDisposable(const sp<Renderer>& self, const sp<Boolean>& disposed = nullptr);
//[[script::bindings::classmethod]]
    static sp<Renderer> makeVisible(const sp<Renderer>& self, const sp<Boolean>& visibility);
//[[script::bindings::classmethod]]
    static sp<Renderer> makeAutoRelease(const sp<Renderer>& self, int32_t refCount = 1);

//[[script::bindings::property]]
    static SafePtr<Size> size(const sp<Renderer>& self);
//[[script::bindings::property]]
    static sp<Renderer> wrapped(const sp<Renderer>& self);
//[[script::bindings::property]]
    static void setWrapped(const sp<Renderer>& self, sp<Renderer> wrapped);

//[[script::bindings::classmethod]]
    static sp<Renderer> translate(const sp<Renderer>& self, const sp<Vec3>& position);

};

}
