#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::class("Renderer")]]
class ARK_API RendererType final {
public:
//  [[script::bindings::enumeration]]
    enum Priority {
        PRIORITY_UI,
        PRIORITY_DEFAULT,
        PRIORITY_UI_BLEND,
        PRIORITY_UI_TEXT,
        PRIORITY_RENDER_LAYER,
        PRIORITY_CONTROL,
        PRIORITY_COUNT
    };

public:
//  [[script::bindings::constructor]]
    static sp<Renderer> create(sp<Renderer> delegate = nullptr);
//  [[script::bindings::constructor]]
    static sp<Renderer> create(Vector<sp<Renderer>> delegate);

//  [[script::bindings::classmethod]]
    static void addRenderer(const sp<Renderer>& self, sp<Renderer> renderer, const Traits& traits);
//  [[script::bindings::classmethod]]
    static sp<Renderer> wrap(sp<Renderer> self);

//  [[script::bindings::classmethod]]
    static sp<Renderer> reset(const sp<Renderer>& self, sp<Renderer> wrapped);

};

}
