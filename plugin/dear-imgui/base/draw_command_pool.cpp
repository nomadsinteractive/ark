#include "dear-imgui/base/draw_command_pool.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

namespace ark::plugin::dear_imgui {

namespace {

PipelineDescriptor::Parameters makePipelineBindingParameters() {
    PipelineDescriptor::PipelineTraitTable traits;
    PipelineDescriptor::TraitConfigure configure;
    configure._cull_face_test = PipelineDescriptor::TraitCullFaceTest{false, PipelineDescriptor::FRONT_FACE_DEFAULT};
    traits.push_back(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST, PipelineDescriptor::PipelineTraitMeta(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST, configure));
    return {Optional<Rect>(), std::move(traits), PipelineDescriptor::FLAG_DYNAMIC_SCISSOR};
}

}

DrawCommandPool::DrawCommandPool(const Shader& shader, const sp<RenderController>& renderController, sp<Texture> texture)
    : _refcount(0), _draw_commands(sp<LFStack<sp<RendererImgui::DrawCommand>>>::make()), _render_controller(renderController),
      _pipeline_bindings(sp<PipelineBindings>::make(Buffer(), shader.pipelineFactory(), sp<PipelineDescriptor>::make(Enum::RENDER_MODE_TRIANGLES, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS, makePipelineBindingParameters(), shader.layout()), std::map<uint32_t, Buffer>{}))
{
    _pipeline_bindings->pipelineDescriptor()->bindSampler(std::move(texture));
}

sp<RendererImgui::DrawCommandRecycler> DrawCommandPool::obtainDrawCommandRecycler()
{
    sp<RendererImgui::DrawCommand> drawCommand;
    if(!_draw_commands->pop(drawCommand))
        drawCommand = sp<RendererImgui::DrawCommand>::make(_render_controller);
    return sp<RendererImgui::DrawCommandRecycler>::make(_draw_commands, drawCommand);
}

}
