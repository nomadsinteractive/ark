#include "dear-imgui/base/draw_command_pool.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

namespace ark::plugin::dear_imgui {

namespace {

const sp<PipelineDescriptor>& makePipelineBindingParameters(const sp<PipelineDescriptor>& pipelineDescriptor) {
    PipelineDescriptor::Configuration configuration = pipelineDescriptor->configuration();
    PipelineDescriptor::PipelineTraitTable& traits = configuration._traits;

    if(!traits.has(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST))
        traits.push_back(PipelineDescriptor::TRAIT_TYPE_CULL_FACE_TEST, {PipelineDescriptor::TraitCullFaceTest{false, PipelineDescriptor::FRONT_FACE_DEFAULT}});
    if(!traits.has(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST))
        traits.push_back(PipelineDescriptor::TRAIT_TYPE_DEPTH_TEST, {PipelineDescriptor::TraitDepthTest{false, false, PipelineDescriptor::COMPARE_FUNC_DEFAULT}});
    traits.push_back(PipelineDescriptor::TRAIT_TYPE_SCISSOR_TEST, {PipelineDescriptor::TraitScissorTest{}});
    pipelineDescriptor->setConfiguration(std::move(configuration));
    return pipelineDescriptor;
}

}

DrawCommandPool::DrawCommandPool(const Shader& shader, const sp<RenderController>& renderController, sp<Texture> texture)
    : _refcount(0), _draw_commands(sp<LFStack<sp<RendererImgui::DrawCommand>>>::make()), _render_controller(renderController),
      _pipeline_bindings(sp<PipelineBindings>::make(enums::DRAW_MODE_TRIANGLES, enums::DRAW_PROCEDURE_DRAW_ELEMENTS, Buffer(), makePipelineBindingParameters(shader.pipelineDesciptor())))
{
    _pipeline_bindings->bindSampler(std::move(texture));
}

sp<RendererImgui::DrawCommandRecycler> DrawCommandPool::obtainDrawCommandRecycler()
{
    Optional<sp<RendererImgui::DrawCommand>> optDrawCommand = _draw_commands->pop();
    sp<RendererImgui::DrawCommand> drawCommand = optDrawCommand ? std::move(optDrawCommand.value()) : sp<RendererImgui::DrawCommand>::make(_render_controller);
    return sp<RendererImgui::DrawCommandRecycler>::make(_draw_commands, drawCommand);
}

}
