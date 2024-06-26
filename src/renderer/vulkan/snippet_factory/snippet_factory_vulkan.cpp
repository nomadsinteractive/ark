#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"

#include "core/util/strings.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"

#include "renderer/inf/snippet.h"

namespace ark::vulkan {

namespace {

class CoreSnippetVulkan final : public Snippet {
public:
    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override {
        ShaderPreprocessor& firstStage = context.stages().begin()->second;

        const String sLocation = "location";

        setLayoutDescriptor(setupLayoutLocation(context, firstStage._declaration_ins), sLocation, 0);

        const PipelineInput& pipelineInput = pipelineLayout.input();

        ShaderPreprocessor* vertex = context.tryGetStage(PipelineInput::SHADER_STAGE_VERTEX);
        if(vertex)
        {
            setLayoutDescriptor(vertex->_declaration_images, "binding", static_cast<uint32_t>(pipelineInput.ubos().size() + pipelineInput.ssbos().size() + pipelineInput.samplerCount()));
            vertex->_predefined_macros.push_back("#define gl_InstanceID gl_InstanceIndex");
        }

        ShaderPreprocessor* fragment = context.tryGetStage(PipelineInput::SHADER_STAGE_FRAGMENT);
        if(fragment)
        {
            fragment->linkNextStage("FragColor");
            setLayoutDescriptor(fragment->_declaration_samplers, "binding", static_cast<uint32_t>(pipelineInput.ubos().size() + pipelineInput.ssbos().size()));
        }

        ShaderPreprocessor* prestage = nullptr;
        for(auto iter = context.stages().begin(); iter != context.stages().end(); ++iter)
        {
            if(iter != context.stages().begin())
            {
                setLayoutDescriptor(prestage->_declaration_outs, iter->second->_declaration_ins, sLocation, 0);
                setLayoutDescriptor(iter->second->_declaration_outs, sLocation, 0);
            }
            prestage = iter->second.get();
        }

        for(const auto& i : context.stages())
        {
            ShaderPreprocessor& preprocessor = i.second;
            preprocessor._version = 450;
            declareUBOStruct(preprocessor, pipelineInput);
            preprocessor._predefined_macros.push_back("#extension GL_ARB_separate_shader_objects : enable");
            preprocessor._predefined_macros.push_back("#extension GL_ARB_shading_language_420pack : enable");
        }
    }

    virtual sp<DrawEvents> makeDrawEvents() override {
        return sp<Snippet::DrawEvents>::make();
    }

private:
    uint32_t getNextLayoutLocation(const ShaderPreprocessor::Declaration& declar, uint32_t& counter) const {
        uint32_t location = counter;
        //TODO: Consider merge uniform type and attribute type into one, something like ShaderDataType.
        Uniform::Type type;
        if(declar.type() == "vec4b")
            type = Uniform::TYPE_I4;
        else if(declar.type() == "vec3b")
            type = Uniform::TYPE_I3;
        else
            type = Uniform::toType(declar.type());
        switch(type)
        {
            case Uniform::TYPE_MAT3:
            case Uniform::TYPE_MAT3V:
                counter += 3 * declar.length();
            break;
            case Uniform::TYPE_MAT4:
            case Uniform::TYPE_MAT4V:
                counter += 4 * declar.length();
            break;
            default:
                counter ++;
        }
        return location;
    }

    std::vector<ShaderPreprocessor::Declaration> setupLayoutLocation(const PipelineBuildingContext& context, const ShaderPreprocessor::DeclarationList& declarations) {
        std::vector<ShaderPreprocessor::Declaration> locations;
        std::map<uint32_t, std::vector<const ShaderPreprocessor::Declaration*>> divisors;

        for(const ShaderPreprocessor::Declaration& i : declarations.vars().values()) {
            const auto iter = context._attributes.find(i.name());
            DCHECK(iter != context._attributes.end(), "Cannot find attribute %s", i.name().c_str());
            const Attribute& attribute = iter->second;
            divisors[attribute.divisor()].push_back(&i);
        }

        for(const auto& i : divisors)
        {
            for(const ShaderPreprocessor::Declaration* j : i.second)
                locations.push_back(*j);
        }

        return locations;
    }

    uint32_t setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& declarations, const String& descriptor, uint32_t start) {
        return setLayoutDescriptor(declarations.vars().values(), descriptor, start);
    }

    uint32_t setLayoutDescriptor(const std::vector<ShaderPreprocessor::Declaration>& declarations, const String& qualifierName, uint32_t start) {
        uint32_t counter = start;
        for(const ShaderPreprocessor::Declaration& i : declarations)
            setLayoutQualifierBinding(i, qualifierName, getNextLayoutLocation(i, counter));
        return counter;
    }

    uint32_t setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& ins, const ShaderPreprocessor::DeclarationList& outs, const String& qualifierName, uint32_t start) {
        uint32_t counter = start;
        DCHECK_WARN(ins.vars().size() == outs.vars().size(), "Output/Input mismatch, output and input have different numbers of items: [%s] vs [%s]",
               Strings::join(&ins.vars().keys().at(0), 0, ins.vars().keys().size()).c_str(), Strings::join(&outs.vars().keys().at(0), 0, outs.vars().keys().size()).c_str());
        for(const ShaderPreprocessor::Declaration& i : ins.vars().values()) {
            uint32_t binding = getNextLayoutLocation(i, counter);
            setLayoutQualifierBinding(i, qualifierName, binding);
            const String outName = Strings::capitalizeFirst(i.name().startsWith("v_") ? i.name().substr(2) : i.name());
            bool hasOutName = outs.vars().has(outName);
            DCHECK_WARN(hasOutName, "Output/Input mismatch, \"%s\" exists in input but not found in next stage of shader", outName.c_str());
            if(hasOutName)
                setLayoutQualifierBinding(outs.vars().at(outName), qualifierName, binding);
        }
        return counter;
    }

    void setLayoutQualifierBinding(const ShaderPreprocessor::Declaration& declaration, const String& qualifierName, uint32_t binding) {
        StringBuffer sb;
        sb << "layout (" << qualifierName << " = " << binding << ") " << *declaration.source();
        *declaration.source() = sb.str();
    }

    void declareUBOStruct(ShaderPreprocessor& shader, const PipelineInput& piplineInput) {
        for(const ShaderPreprocessor::Declaration& i : shader._declaration_uniforms.vars().values())
            *i.source() = "";

        for(const sp<PipelineInput::UBO>& i : piplineInput.ubos())
            if(hasUBO(shader, i))
                insertUBOStruct(shader, i);
    }

    void insertUBOStruct(ShaderPreprocessor& shader, PipelineInput::UBO& ubo) const {
        StringBuffer sb;
        sb << "layout (binding = " << ubo.binding() << ") uniform UBO" << ubo.binding() << " {\n";
        for(const auto& i : ubo.uniforms().values()) {
            shader._main.replace(i->name(), Strings::sprintf("ubo%d.%s", ubo.binding(), i->name().c_str()));
            sb << i->declaration("") << '\n';
        }
        sb << "} ubo" << ubo.binding() << ";\n\n";
        shader._uniform_declaration_codes.push_back(sp<String>::make(sb.str()));
    }

    bool hasUBO(ShaderPreprocessor& shader, PipelineInput::UBO& ubo) const {
        for(const auto& i : ubo.uniforms().values())
            if(shader._main.contains(i->name()))
                return true;
        return false;
    }

};

}

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet()
{
    return sp<CoreSnippetVulkan>::make();
}

}