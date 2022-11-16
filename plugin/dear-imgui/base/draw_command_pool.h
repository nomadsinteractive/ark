#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_DRAW_COMMAND_POOL_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_DRAW_COMMAND_POOL_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class DrawCommandPool {
public:
    DrawCommandPool(const Shader& shader, const sp<RenderController>& renderController, sp<Texture> texture);

    sp<RendererImgui::DrawCommandRecycler> obtainDrawCommandRecycler();

    int32_t _refcount;

    sp<LFStack<sp<RendererImgui::DrawCommand>>> _draw_commands;

    sp<RenderController> _render_controller;
    sp<ShaderBindings> _shader_bindings;
};

}
}
}

#endif
