#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark::plugin::dear_imgui {

class DrawCommandPool {
public:
    DrawCommandPool(const Shader& shader, const sp<RenderController>& renderController, sp<Texture> texture);

    sp<RendererImgui::DrawCommandRecycler> obtainDrawCommandRecycler();

    int32_t _refcount;

    sp<LFStack<sp<RendererImgui::DrawCommand>>> _draw_commands;

    sp<RenderController> _render_controller;
    sp<PipelineBindings> _pipeline_bindings;
};

}
