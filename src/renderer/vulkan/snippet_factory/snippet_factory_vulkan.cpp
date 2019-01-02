#include "renderer/vulkan/snippet_factory/snippet_factory_vulkan.h"

#include "renderer/base/resource_manager.h"
#include "renderer/base/pipeline_building_context.h"

#include "renderer/inf/snippet.h"

namespace ark {
namespace vulkan {

namespace {

class CoreSnippetVulkan : public Snippet {
public:
    virtual void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const sp<ShaderBindings>& /*shaderBindings*/) override {
        context._vertex._version = 450;
        context._fragment._version = 450;

        const String sLocation = "location";
        const String sBinding = "binding";

        setLayoutDescriptor(context._vertex._ins.vars().values(), sLocation, 0);
        setLayoutDescriptor(context._vertex._outs.vars().values(), sLocation, 0);

        for(const auto& i : context._vertex._uniforms.vars().values())
        {
            *i.source() = i.source()->replace("uniform ", "    ");
            context._vertex._main.replace(i.name(), "ubo." + i.name());
        }

        context._fragment._outs.declare("vec4", "v_", "FragColor");

        setLayoutDescriptor(context._fragment._samplers.vars().values(), sBinding, 1);

        setLayoutDescriptor(context._fragment._ins.vars().values(), sLocation, 0);
        setLayoutDescriptor(context._fragment._outs.vars().values(), sLocation, 0);

        context._vertex._uniform_declarations.push_front(sp<String>::make("layout (binding = 0) uniform UBO {\n"));
        context._vertex._uniform_declarations.push_back(sp<String>::make("\n} ubo;\n\n"));

        context._vertex._macro_defines.push_back("#extension GL_ARB_separate_shader_objects : enable");
        context._vertex._macro_defines.push_back("#extension GL_ARB_shading_language_420pack : enable");

        context._fragment._macro_defines.push_back("#extension GL_ARB_separate_shader_objects : enable");
        context._fragment._macro_defines.push_back("#extension GL_ARB_shading_language_420pack : enable");

        context._fragment._macro_defines.push_back("#define texture2D texture");
        context._fragment._macro_defines.push_back("#define textureCube texture");
    }

private:
    void setLayoutDescriptor(const std::vector<ShaderPreprocessor::Declaration>& declarations, const String& descriptor, uint32_t start)
    {
        uint32_t counter = start;
        for(const ShaderPreprocessor::Declaration& i : declarations)
        {
            StringBuffer sb;
            sb << "layout (" << descriptor << " = " << (counter++) << ") " << *i.source();
            *i.source() = sb.str();
        }
    }
};

}

sp<Snippet> SnippetFactoryVulkan::createCoreSnippet(ResourceManager& /*resourceManager*/, const sp<PipelineFactory>& /*pipelineFactory*/, const sp<ShaderBindings>& /*shaderBindings*/)
{
    return sp<CoreSnippetVulkan>::make();
}

}
}
