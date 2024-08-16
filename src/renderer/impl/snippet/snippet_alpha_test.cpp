#include "renderer/impl/snippet/snippet_alpha_test.h"

#include "core/util/strings.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

SnippetAlphaTest::SnippetAlphaTest(float threshold)
    : _threshold(threshold)
{
}

void SnippetAlphaTest::preInitialize(PipelineBuildingContext& context)
{
    ShaderPreprocessor& preprocessor = context.getStage(ShaderStage::SHADER_STAGE_FRAGMENT);
    preprocessor.addPostMainSource(Strings::sprintf("if(%s.a < %.2f)\n        discard;", preprocessor.outputName().c_str(), _threshold));
}

SnippetAlphaTest::BUILDER::BUILDER(BeanFactory& /*factory*/, const document& manifest)
    : _threshold(Documents::ensureAttribute<float>(manifest, "threshold"))
{
}

sp<Snippet> SnippetAlphaTest::BUILDER::build(const Scope& /*args*/)
{
    return sp<SnippetAlphaTest>::make(_threshold);
}

}
