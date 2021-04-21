#include "renderer/impl/snippet/snippet_acolor.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

void SnippetAcolor::preInitialize(PipelineBuildingContext& context)
{
    ShaderPreprocessor& fragment = context.getStage(PipelineInput::SHADER_STAGE_FRAGMENT);
    context.addPredefinedAttribute("Color", "vec4", PipelineInput::SHADER_STAGE_FRAGMENT);
    fragment.addOutputVarModifier("v_Color");
}

sp<Snippet> SnippetAcolor::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<SnippetAcolor>::make();
}

}
