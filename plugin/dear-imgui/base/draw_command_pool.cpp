#include "dear-imgui/base/draw_command_pool.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

DrawCommandPool::DrawCommandPool(const Shader& shader, const sp<RenderController>& renderController, sp<Texture> texture)
    : _refcount(0), _draw_commands(sp<LFStack<sp<RendererImgui::DrawCommand>>>::make()), _render_controller(renderController),
      _shader_bindings(sp<ShaderBindings>::make(Buffer(), shader.pipelineFactory(), sp<PipelineBindings>::make(ModelLoader::RENDER_MODE_TRIANGLES, PipelineBindings::DRAW_PROCEDURE_DRAW_ELEMENTS, PipelineBindings::Parameters(Optional<Rect>(), PipelineBindings::PipelineTraitTable(), PipelineBindings::FLAG_CULL_MODE_NONE | PipelineBindings::FLAG_DYNAMIC_SCISSOR), shader.layout()), std::map<uint32_t, Buffer>{}))
{
    _shader_bindings->pipelineBindings()->bindSampler(std::move(texture));
}

sp<RendererImgui::DrawCommandRecycler> DrawCommandPool::obtainDrawCommandRecycler()
{
    sp<RendererImgui::DrawCommand> drawCommand;
    if(!_draw_commands->pop(drawCommand))
        drawCommand = sp<RendererImgui::DrawCommand>::make(_render_controller);
    return sp<RendererImgui::DrawCommandRecycler>::make(_draw_commands, drawCommand);
}

}
}
}
