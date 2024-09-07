#include "dear-imgui/base/imgui_context.h"

namespace ark::plugin::dear_imgui {

ImguiContext::ImguiContext()
    : _context(ImGui::CreateContext()) {
}

ImguiContext::~ImguiContext()
{
    ImGui::DestroyContext(_context);
}

}
