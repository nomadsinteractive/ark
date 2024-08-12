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
    void preInitialize(PipelineBuildingContext& context) override {
        context.addAttribute("Alpha01", "float", 0);
        context.addAttribute("PointSize", "float", 0);
        context.addUniform("u_Color01", Uniform::TYPE_F4, 1, nullptr, 0);
    }

    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineLayout& /*pipelineLayout*/) override {
        ShaderPreprocessor& vertex = context.getStage(PipelineInput::SHADER_STAGE_VERTEX);
        ShaderPreprocessor& fragment = context.getStage(PipelineInput::SHADER_STAGE_FRAGMENT);

        fragment.addOutputModifier("", " * vec4(1.0, 1.0, 1.0, v_Alpha01)");
        fragment.addOutputModifier("u_Color01", "");
        vertex.addPostMainSource("gl_PointSize = a_PointSize;");
    }
};

}

class PipelinesTestCase : public TestCase {
public:
    virtual int launch() {
        Global<StringTable> stringTable;
        const sp<String> vert = stringTable->getString("shaders", "default.vert", true);
        const sp<String> frag = stringTable->getString("shaders", "texture.frag", true);
        if(!vert || !frag)
            return -1;

        const sp<PipelineFactory> pipelineFactory = Ark::instance().applicationContext()->renderEngine()->rendererFactory()->createPipelineFactory();
        const sp<Snippet> snippet = sp<SnippetTest>::make();
        const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(Ark::instance().applicationContext()->renderController(), nullptr, std::move(vert), std::move(frag));
        buildingContext->addSnippet(snippet);

        const sp<PipelineLayout> pipelineLayout = sp<PipelineLayout>::make(buildingContext, buildingContext->makePipelineSnippet());
        const sp<PipelineInput>& pipelineInput = pipelineLayout->input();

        TESTCASE_VALIDATE(pipelineInput->layouts()[0].stride() != 0);
        TESTCASE_VALIDATE(pipelineInput->getAttribute("Position")->length());
        TESTCASE_VALIDATE(pipelineInput->getAttribute("TexCoordinate")->offset() != -1);
        TESTCASE_VALIDATE(pipelineInput->getAttribute("Alpha01")->offset() == -1);

        return 0;
    }
};

}
}

ark::unittest::TestCase* pipelines_create() {
    return new ark::unittest::PipelinesTestCase();
}
