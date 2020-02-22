#include "dear-imgui/base/imgui_context.h"


namespace ark {
namespace plugin {
namespace dear_imgui {

ImguiContext::ImguiContext()
    : _context(ImGui::CreateContext()) {
}

ImguiContext::~ImguiContext()
{
    ImGui::DestroyContext(_context);
}

}
}
}
