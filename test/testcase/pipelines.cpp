#include "test/base/test_case.h"

#include "core/base/string_table.h"
#include "core/inf/dictionary.h"
#include "core/types/global.h"

#include "renderer/base/pipeline_building_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/shader_preprocessor.h"
#include "renderer/base/uniform.h"
#include "renderer/inf/snippet.h"

#include "app/base/application_context.h"
#include "renderer/inf/renderer_factory.h"

namespace ark::unittest {

namespace {

class SnippetTest : public Snippet {
public:
    void preInitialize(PipelineBuildingContext& context) override {
        context.addAttribute("Alpha01", "float", 0);
        context.addAttribute("PointSize", "float", 0);
        context.addUniform("u_Color01", Uniform::TYPE_F4, 1, nullptr);
    }

    void preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context, const PipelineDescriptor& /*pipelineDescriptor*/) override {
        ShaderPreprocessor& vertex = context.getRenderStage(enums::SHADER_STAGE_BIT_VERTEX);
        ShaderPreprocessor& fragment = context.getRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT);

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
        const Optional<String> vert = stringTable->getString("shaders", "default.vert", true);
        const Optional<String> frag = stringTable->getString("shaders", "texture.frag", true);
        if(!vert || !frag)
            return -1;

        const sp<PipelineFactory> pipelineFactory = Ark::instance().applicationContext()->renderEngine()->rendererFactory()->createPipelineFactory();
        const sp<Snippet> snippet = sp<SnippetTest>::make();
        const sp<PipelineBuildingContext> buildingContext = sp<PipelineBuildingContext>::make(vert.value(), frag.value());

        const sp<PipelineDescriptor> pipelineDescriptor = sp<PipelineDescriptor>::make(Camera::createDefaultCamera(), buildingContext, PipelineDescriptor::Configuration{{}, nullptr, nullptr, std::move(snippet)});
        const sp<PipelineLayout>& pipelineLayout = pipelineDescriptor->layout();

        TESTCASE_VALIDATE(pipelineLayout->getStreamLayout(0).stride() != 0);
        TESTCASE_VALIDATE(pipelineLayout->getAttribute("Position")->length());
        TESTCASE_VALIDATE(pipelineLayout->getAttribute("TexCoordinate")->offset() != -1);
        TESTCASE_VALIDATE(pipelineLayout->getAttribute("Alpha01")->offset() == -1);

        return 0;
    }
};

}

ark::unittest::TestCase* pipelines_create() {
    return new ark::unittest::PipelinesTestCase();
}
