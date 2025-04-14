#include "renderer/impl/snippet/snippet_alpha.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

void SnippetAlpha::preInitialize(PipelineBuildingContext& context)
{
    ShaderPreprocessor& fragment = context.getRenderStage(enums::SHADER_STAGE_BIT_FRAGMENT);
    context.addPredefinedAttribute("Alpha", "float", 0, enums::SHADER_STAGE_BIT_FRAGMENT);
    fragment.addOutputModifier("", " * vec4(1.0, 1.0, 1.0, v_Alpha)");
}

sp<Snippet> SnippetAlpha::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<SnippetAlpha>::make();
}

}
