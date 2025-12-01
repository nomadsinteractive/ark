#include "vulkan/impl/snippet_factory/snippet_factory_vulkan.h"

#include <ranges>

#include "core/util/strings.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_layout.h"

#include "renderer/inf/snippet.h"
#include "renderer/util/render_util.h"

namespace ark::plugin::vulkan {

namespace {

class CoreSnippetVulkan final : public Snippet {
public:
    void postInitialize(PipelineBuildingContext& context) override
    {
        const String sLocation = "location";

        if(!context.renderStages().empty())
        {
            const ShaderPreprocessor& firstStage = context.renderStages().begin()->second;
            RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);
        }

        const PipelineLayout& pipelineLayout = context._pipeline_layout;

        if(ShaderPreprocessor* vertex = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_VERTEX))
        {
            CHECK(vertex->_ssbos.empty(), "SSBO should not be declared in vertex shaders");
            RenderUtil::setLayoutDescriptor(vertex->_declaration_images, "binding", pipelineLayout.samplers().size(), 3);
            vertex->_predefined_macros.emplace_back("#define gl_InstanceID gl_InstanceIndex");
        }
        if(ShaderPreprocessor* fragment = context.tryGetRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT))
        {
            int32_t ssboBindingLocation = 0;
            fragment->linkNextStage("FragColor");
            RenderUtil::setLayoutDescriptor(fragment->_declaration_samplers, "binding", 0, 0);
            RenderUtil::setLayoutDescriptor(fragment->_declaration_images, "binding", static_cast<uint32_t>(fragment->_declaration_samplers.vars().size()), 3);
            for(auto& [k, v] : fragment->_ssbos)
                RenderUtil::overrideLayoutDescriptor(k, v._declaration, v._binding, ssboBindingLocation++, 2, "Storage buffer", "fragment shaders");
        }
        if(ShaderPreprocessor* compute = context.computingStage().get())
        {
            int32_t ssboBindingLocation = 0;
            RenderUtil::setLayoutDescriptor(compute->_declaration_samplers, "binding", 0, 0);
            RenderUtil::setLayoutDescriptor(compute->_declaration_images, "binding", static_cast<uint32_t>(compute->_declaration_samplers.vars().size()), 3);
            for(auto& [k, v] : compute->_ssbos)
                RenderUtil::overrideLayoutDescriptor(k, v._declaration, v._binding, ssboBindingLocation++, 2, "Storage buffer", "compute shaders");
        }

        const ShaderPreprocessor* prestage = nullptr;
        for(const op<ShaderPreprocessor>& stage : context.renderStages() | std::views::values)
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
            preprocessor->_predefined_macros.emplace_back("#define ARK_USE_VULKAN");
        }
    }
};

}

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet()
{
    return sp<CoreSnippetVulkan>::make();
}

}