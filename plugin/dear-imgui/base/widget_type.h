#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_WIDGET_TYPE_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_WIDGET_TYPE_H_

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

//[[script::bindings::class("Widget")]]
class ARK_PLUGIN_DEAR_IMGUI_API WidgetType final {
public:
//  [[script::bindings::constructor]]
    static sp<Widget> create(sp<Widget> wrapped = nullptr);

//  [[script::bindings::classmethod]]
    static sp<Widget> makeVisible(sp<Widget> self, sp<Boolean> visibility);
//  [[script::bindings::classmethod]]
    static sp<Widget> before(sp<Widget> self, sp<Widget> after);

//  [[script::bindings::property]]
    static sp<Widget> wrapped(sp<Widget> self);
//  [[script::bindings::property]]
    static void setWrapped(const sp<Widget>& self, sp<Widget> wrapped);

//  [[script::bindings::classmethod]]
    static sp<Renderer> toRenderer(sp<Widget> self);

};

}
}
}

#endif
