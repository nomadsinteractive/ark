#pragma once

#include <imgui.h>

#include "core/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"

namespace ark::plugin::dear_imgui {

class ARK_PLUGIN_DEAR_IMGUI_API Imgui {
public:
//  [[script::bindings::enumeration]]
    enum ImGuiDataType {
        ImGuiDataType_S8 = ImGuiDataType_::ImGuiDataType_S8,
        ImGuiDataType_U8 = ImGuiDataType_::ImGuiDataType_U8,
        ImGuiDataType_S16 = ImGuiDataType_::ImGuiDataType_S16,
        ImGuiDataType_U16 = ImGuiDataType_::ImGuiDataType_U16,
        ImGuiDataType_S32 = ImGuiDataType_::ImGuiDataType_S32,
        ImGuiDataType_U32 = ImGuiDataType_::ImGuiDataType_U32,
        ImGuiDataType_S64 = ImGuiDataType_::ImGuiDataType_S64,
        ImGuiDataType_U64 = ImGuiDataType_::ImGuiDataType_U64,
        ImGuiDataType_Float = ImGuiDataType_::ImGuiDataType_Float,
        ImGuiDataType_Double = ImGuiDataType_::ImGuiDataType_Double,
        ImGuiDataType_Bool = ImGuiDataType_::ImGuiDataType_Bool,
        ImGuiDataType_String = ImGuiDataType_::ImGuiDataType_String,
        ImGuiDataType_COUNT = ImGuiDataType_::ImGuiDataType_COUNT
    };
};

}
