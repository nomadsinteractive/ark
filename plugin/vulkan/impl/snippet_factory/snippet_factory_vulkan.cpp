#include "vulkan/impl/snippet_factory/snippet_factory_vulkan.h"

#include "core/util/strings.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_configuration.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"

#include "renderer/inf/snippet.h"
#include "renderer/util/render_util.h"

namespace ark::plugin::vulkan {

namespace {

class CoreSnippetVulkan final : public Snippet {
public:
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineDescriptor& pipelineDescriptor) override {
        const String sLocation = "location";

        if(!context.renderStages().empty())
        {
            const ShaderPreprocessor& firstStage = context.renderStages().begin()->second;
            RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);
        }

        const PipelineLayout& pipelineLayout = pipelineDescriptor.layout();
        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_VERTEX))
        {
            RenderUtil::setLayoutDescriptor(vertex->_declaration_images, "binding", static_cast<uint32_t>(pipelineLayout.ssbos().size() + pipelineLayout.samplers().size()));
            vertex->_predefined_macros.emplace_back("#define gl_InstanceID gl_InstanceIndex");
        }
        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(Enum::SHADER_STAGE_BIT_FRAGMENT))
        {
            fragment->linkNextStage("FragColor");
            const uint32_t bindingOffset = static_cast<uint32_t>(pipelineLayout.ssbos().size());
            RenderUtil::setLayoutDescriptor(fragment->_declaration_samplers, "binding", bindingOffset, 2);
            RenderUtil::setLayoutDescriptor(fragment->_declaration_images, "binding", bindingOffset + static_cast<uint32_t>(fragment->_declaration_samplers.vars().size()), 2);
        }

        if(const ShaderPreprocessor* compute = context.computingStage().get())
        {
            const uint32_t bindingOffset = static_cast<uint32_t>(pipelineLayout.ssbos().size());
            RenderUtil::setLayoutDescriptor(compute->_declaration_images, "binding", bindingOffset, 2);
        }

        const ShaderPreprocessor* prestage = nullptr;
        for(const auto& [_, stage] : context.renderStages())
        {
            if(prestage)
            {
                RenderUtil::setLayoutDescriptor(prestage->_declaration_outs, stage->_declaration_ins, sLocation, 0);
                RenderUtil::setLayoutDescriptor(stage->_declaration_outs, sLocation, 0);
            }
            prestage = stage.get();
        }

        for(ShaderPreprocessor* preprocessor : context.stages())
        {
            preprocessor->_version = 450;
            preprocessor->declareUBOStruct(pipelineLayout, 1);
            preprocessor->_predefined_macros.emplace_back("#extension GL_ARB_separate_shader_objects : enable");
            preprocessor->_predefined_macros.emplace_back("#extension GL_ARB_shading_language_420pack : enable");
        }
    }

};

}

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet()
{
    return sp<CoreSnippetVulkan>::make();
}

}