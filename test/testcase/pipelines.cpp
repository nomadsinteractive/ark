#include "test/base/test_case.h"

#include "core/base/string_table.h"
#include "core/inf/dictionary.h"
#include "core/types/global.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/pipeline_layout.h"
#include "renderer/base/uniform.h"
#include "renderer/inf/snippet.h"

#include "app/base/application_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark {
namespace unittest {

namespace {

class SnippetTest : public Snippet {
public:
    virtual void preInitialize(PipelineBuildingContext& context) override {
        context.addAttribute("Alpha01", "float");
        context.addAttribute("PointSize", "float");
        context.addUniform("u_Color01", Uniform::TYPE_F4, nullptr, nullptr, 0);
    }

    virtual void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const sp<ShaderBindings>& /*shaderBindings*/) override {
        context._fragment.addModifier("vec4(1.0, 1.0, 1.0, v_Alpha01)");
        context._fragment.addModifier("u_Color01");
        context._vertex.addSource("gl_PointSize = a_PointSize;");
    }
};

}

class PipelinesTestCase : public TestCase {
public:
    virtual int launch() {
        Global<StringTable> stringTable;
        const sp<String> vert = stringTable->getString("shaders", "default.vert");
        const sp<String> frag = stringTable->getString("shaders", "texture.frag");
        if(!vert || !frag)
            return -1;

        const sp<PipelineFactory> pipelineFactory = Ark::instance().applicationContext()->renderEngine()->rendererFactory()->createPipelineFactory();
        const sp<Snippet> snippet = sp<SnippetTest>::make();
        const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(pipelineFactory, vert, frag);
        buildingContext->addSnippet(snippet);

        const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(Ark::instance().applicationContext()->renderController(), buildingContext);
        const sp<Shader> shader = sp<Shader>::make(buildingContext->_shader, pipelineLayout, nullptr);
        const sp<PipelineInput>& pipelineInput = shader->pipelineLayout()->input();
//        puts(source->vertex()._source.c_str());
        puts("---------------------------------------------------------------------");
//        puts(source->fragment()._source.c_str());
        if(pipelineInput->streams()[0].stride() == 0)
            return 1;
        if(!pipelineInput->getAttribute("Position").length())
            return 2;
        if(!pipelineInput->getAttribute("TexCoordinate").offset())
            return 3;
        if(!pipelineInput->getAttribute("Alpha01").offset())
            return 4;

        return 0;
    }
};

}
}

ark::unittest::TestCase* pipelines_create() {
    return new ark::unittest::PipelinesTestCase();
}