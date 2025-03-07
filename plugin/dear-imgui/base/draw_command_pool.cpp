#include "dear-imgui/base/draw_command_pool.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

namespace ark::plugin::dear_imgui {

namespace {

sp<PipelineDescriptor> makePipelineBindingParameters(const PipelineDescriptor& pipelineDescriptor) {
    PipelineDescriptor::Configuration parameters = pipelineDescriptor.configuration();
    PipelineDescriptor::PipelineTraitTable& traits = parameters._traits;
    if(!traits.has(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST))
    {
        PipelineDescriptor::TraitConfigure configure;
        configure._cull_face_test = PipelineDescriptor::TraitCullFaceTest{false, PipelineDescriptor::FRONT_FACE_DEFAULT};
        traits.push_back(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST, configure);
    }
    if(!traits.has(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST))
    {
        PipelineDescriptor::TraitConfigure configure;
        configure._depth_test = PipelineDescriptor::TraitDepthTest{false, false, PipelineDescriptor::COMPARE_FUNC_DEFAULT};
        traits.push_back(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST, configure);
    }
    traits.push_back(PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST, {});

    sp<PipelineDescriptor> pd = sp<PipelineDescriptor>::make(pipelineDescriptor);
    pd->setParameters(std::move(parameters));
    return pd;
}

}

DrawCommandPool::DrawCommandPool(const Shader& shader, const sp<RenderController>& renderController, sp<Texture> texture)
    : _refcount(0), _draw_commands(sp<LFStack<sp<RendererImgui::DrawCommand>>>::make()), _render_controller(renderController),
      _pipeline_bindings(sp<PipelineBindings>::make(Enum::DRAW_MODE_TRIANGLES, Enum::DRAW_PROCEDURE_DRAW_ELEMENTS, Buffer(), makePipelineBindingParameters(shader.pipelineDesciptor()), Map<uint32_t, Buffer>{}))
{
    _pipeline_bindings->bindSampler(std::move(texture));
}

sp<RendererImgui::DrawCommandRecycler> DrawCommandPool::obtainDrawCommandRecycler()
{
    sp<RendererImgui::DrawCommand> drawCommand;
    if(!_draw_commands->pop(drawCommand))
        drawCommand = sp<RendererImgui::DrawCommand>::make(_render_controller);
    return sp<RendererImgui::DrawCommandRecycler>::make(_draw_commands, drawCommand);
}

}
