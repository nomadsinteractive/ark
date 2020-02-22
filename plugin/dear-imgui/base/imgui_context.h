#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_IMGUI_CONTEXT_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_IMGUI_CONTEXT_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include <imgui.h>

namespace ark {
namespace plugin {
namespace dear_imgui {

class ImguiContext {
public:
    ImguiContext();
    ~ImguiContext();

private:
    ImGuiContext* _context;
};

}
}
}

#endif
