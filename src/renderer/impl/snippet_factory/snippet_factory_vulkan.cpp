#include "renderer/impl/snippet_factory/snippet_factory_vulkan.h"

#include "core/util/strings.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"

#include "renderer/inf/snippet.h"
#include "renderer/util/render_util.h"

namespace ark {

namespace {

void insertUBOStruct(ShaderPreprocessor& shader, const PipelineInput::UBO& ubo)
{
    StringBuffer sb;
    sb << "layout (binding = " << ubo.binding() << ") uniform UBO" << ubo.binding() << " {\n";
    for(const auto& i : ubo.uniforms().values()) {
        shader._main.replace(i->name(), Strings::sprintf("ubo%d.%s", ubo.binding(), i->name().c_str()));
        sb << i->declaration("") << '\n';
    }
    sb << "} ubo" << ubo.binding() << ";\n\n";
    shader._uniform_declaration_codes.push_back(sp<String>::make(sb.str()));
}

bool hasUBO(const ShaderPreprocessor& shader, const PipelineInput::UBO& ubo)
{
    for(const auto& i : ubo.uniforms().values())
        if(shader._main.contains(i->name()))
            return true;
    return false;
}

void declareUBOStruct(ShaderPreprocessor& shader, const PipelineInput& piplineInput)
{
    for(const ShaderPreprocessor::Declaration& i : shader._declaration_uniforms.vars().values())
        i.setSource("");

    for(const sp<PipelineInput::UBO>& i : piplineInput.ubos())
        if(hasUBO(shader, i))
            insertUBOStruct(shader, i);
}

class CoreSnippetVulkan final : public Snippet {
public:
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override {
        const String sLocation = "location";
        const ShaderPreprocessor& firstStage = context.stages().begin()->second;

        RenderUtil::setLayoutDescriptor(RenderUtil::setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);

        const PipelineInput& pipelineInput = pipelineLayout.input();
        if(ShaderPreprocessor* vertex = context.tryGetStage(PipelineInput::SHADER_STAGE_VERTEX))
        {
            RenderUtil::setLayoutDescriptor(vertex->_declaration_images, "binding", static_cast<uint32_t>(pipelineInput.ubos().size() + pipelineInput.ssbos().size() + pipelineInput.samplerCount()));
            vertex->_predefined_macros.push_back("#define gl_InstanceID gl_InstanceIndex");
        }
        if(ShaderPreprocessor* fragment = context.tryGetStage(PipelineInput::SHADER_STAGE_FRAGMENT))
        {
            fragment->linkNextStage("FragColor");
            const uint32_t bindingOffset = static_cast<uint32_t>(pipelineInput.ubos().size() + pipelineInput.ssbos().size());
            RenderUtil::setLayoutDescriptor(fragment->_declaration_samplers, "binding", bindingOffset);
            RenderUtil::setLayoutDescriptor(fragment->_declaration_images, "binding", bindingOffset + static_cast<uint32_t>(fragment->_declaration_samplers.vars().size()));
        }

        if(const ShaderPreprocessor* compute = context.tryGetStage(PipelineInput::SHADER_STAGE_COMPUTE))
        {
            const uint32_t bindingOffset = static_cast<uint32_t>(pipelineInput.ubos().size() + pipelineInput.ssbos().size());
            RenderUtil::setLayoutDescriptor(compute->_declaration_images, "binding", bindingOffset);
        }

        const ShaderPreprocessor* prestage = nullptr;
        for(auto iter = context.stages().begin(); iter != context.stages().end(); ++iter)
        {
            if(iter != context.stages().begin())
            {
                RenderUtil::setLayoutDescriptor(prestage->_declaration_outs, iter->second->_declaration_ins, sLocation, 0);
                RenderUtil::setLayoutDescriptor(iter->second->_declaration_outs, sLocation, 0);
            }
            prestage = iter->second.get();
        }

        for(const auto& [_, v] : context.stages())
        {
            ShaderPreprocessor& preprocessor = v;
            preprocessor._version = 450;
            declareUBOStruct(preprocessor, pipelineInput);
            preprocessor._predefined_macros.push_back("#extension GL_ARB_separate_shader_objects : enable");
            preprocessor._predefined_macros.push_back("#extension GL_ARB_shading_language_420pack : enable");
        }
    }

    sp<DrawEvents> makeDrawEvents() override {
        return sp<Snippet::DrawEvents>::make();
    }

};

}

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet()
{
    return sp<CoreSnippetVulkan>::make();
}

}