#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"

#include "core/util/strings.h"

#include "renderer/base/render_controller.h"
#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/pipeline_input.h"

#include "renderer/inf/snippet.h"

namespace ark {
namespace vulkan {

namespace {

class CoreSnippetVulkan : public Snippet {
public:
    virtual void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& pipelineLayout) override {
        context._vertex._version = 450;
        context._fragment._version = 450;

        const String sLocation = "location";
        const String sBinding = "binding";

        setLayoutDescriptor(setupLayoutLocation(context, context._vertex._ins), sLocation, 0);

        const sp<PipelineInput>& pipelineInput = pipelineLayout.input();
        declareUBOStruct(context._vertex, pipelineInput);
        declareUBOStruct(context._fragment, pipelineInput);

        context._fragment._outs.declare("vec4", "v_", "FragColor");

        setLayoutDescriptor(context._fragment._samplers, sBinding, static_cast<uint32_t>(pipelineInput->ubos().size()));

        setLayoutDescriptor(context._vertex._outs, context._fragment._ins, sLocation, 0);
        setLayoutDescriptor(context._fragment._outs, sLocation, 0);

        context._vertex._macro_defines.push_back("#extension GL_ARB_separate_shader_objects : enable");
        context._vertex._macro_defines.push_back("#extension GL_ARB_shading_language_420pack : enable");

        context._fragment._macro_defines.push_back("#extension GL_ARB_separate_shader_objects : enable");
        context._fragment._macro_defines.push_back("#extension GL_ARB_shading_language_420pack : enable");

        context._fragment._macro_defines.push_back("#define texture2D texture");
        context._fragment._macro_defines.push_back("#define textureCube texture");
    }

private:
    uint32_t getNextLayoutLocation(const ShaderPreprocessor::Declaration& declar, uint32_t& counter) const {
        uint32_t location = counter;
        Uniform::Type type = Uniform::toType(declar.type());
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

    uint32_t setLayoutDescriptor(const std::vector<ShaderPreprocessor::Declaration>& declarations, const String& descriptor, uint32_t start) {
        uint32_t counter = start;
        for(const ShaderPreprocessor::Declaration& i : declarations) {
            StringBuffer sb;
            sb << "layout (" << descriptor << " = " << getNextLayoutLocation(i, counter) << ") " << *i.source();
            *i.source() = sb.str();
        }
        return counter;
    }

    uint32_t setLayoutDescriptor(const ShaderPreprocessor::DeclarationList& ins, const ShaderPreprocessor::DeclarationList& outs, const String& descriptor, uint32_t start) {
        uint32_t counter = start;
        DCHECK(ins.vars().size() == outs.vars().size(), "Output/Input mismatch, output and input have different numbers of items: [%s] vs [%s]", Strings::join(&ins.vars().keys().at(0), 0, ins.vars().keys().size()).c_str(), Strings::join(&outs.vars().keys().at(0), 0, outs.vars().keys().size()).c_str());
        for(const ShaderPreprocessor::Declaration& i : ins.vars().values()) {
            const String prefix = Strings::sprintf("layout (%s = %d) ", descriptor.c_str(), getNextLayoutLocation(i, counter));
            *i.source() = prefix + *i.source();

            DCHECK(outs.vars().has(i.name()), "Output/Input mismatch, \"%s\" exists in input but not found in next stage of shader", i.name().c_str());
            const sp<String>& os = outs.vars().at(i.name()).source();
            *os = prefix + *os;
        }
        return counter;
    }

    void declareUBOStruct(ShaderPreprocessor& shader, const PipelineInput& piplineInput) {
        for(const ShaderPreprocessor::Declaration& i : shader._uniforms.vars().values())
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
        shader._uniform_declarations.push_back(sp<String>::make(sb.str()));
    }

    bool hasUBO(ShaderPreprocessor& shader, PipelineInput::UBO& ubo) const {
        for(const auto& i : ubo.uniforms().values())
            if(shader._main.contains(i->name()))
                return true;
        return false;
    }
};

}

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet(RenderController& /*resourceManager*/)
{
    return sp<CoreSnippetVulkan>::make();
}

}
}
