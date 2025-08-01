#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"

namespace ark::plugin::dear_imgui {

//[[script::bindings::class("Widget")]]
class ARK_PLUGIN_DEAR_IMGUI_API WidgetType final {
public:
//  [[script::bindings::constructor]]
    static sp<Widget> create(sp<Widget> delegate = nullptr);
//  [[script::bindings::constructor]]
    static sp<Widget> create(Vector<sp<Widget>> delegate);

//  [[script::bindings::classmethod]]
    static void visibleIf(const sp<Widget>& self, sp<Boolean> visibility);

//  [[script::bindings::classmethod]]
    static void reset(const sp<Widget>& self, sp<Widget> wrapped = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Renderer> toRenderer(sp<Widget> self);
};

}
