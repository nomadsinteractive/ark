#pragma once

#include "renderer/forwarding.h"

#include <imgui.h>

namespace ark::plugin::dear_imgui {

class ImguiContext {
public:
    ImguiContext();
    ~ImguiContext();

private:
    ImGuiContext* _context;
};

}
