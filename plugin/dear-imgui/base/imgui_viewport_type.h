#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"

namespace ark::plugin::dear_imgui {

class ARK_PLUGIN_DEAR_IMGUI_API ImGuiViewportType {
public:
    ImGuiViewportType(ImGuiViewport* viewport);

//  [[script::bindings::property]]
    sp<Integer> id() const;

//  [[script::bindings::property]]
    sp<Vec2> pos() const;
//  [[script::bindings::property]]
    sp<Vec2> size() const;
//  [[script::bindings::property]]
    sp<Vec2> workPos() const;
//  [[script::bindings::property]]
    sp<Vec2> workSize() const;

private:
    ::ImGuiViewport* _imgui_viewport;
};

}
