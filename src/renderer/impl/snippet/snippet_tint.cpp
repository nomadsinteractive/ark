#include "renderer/impl/snippet/snippet_tint.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

void SnippetTint::preInitialize(PipelineBuildingContext& context)
{
    ShaderPreprocessor& fragment = context.getStage(PipelineInput::SHADER_STAGE_FRAGMENT);
    context.addPredefinedAttribute("Color", "vec4", 0, PipelineInput::SHADER_STAGE_FRAGMENT);
    fragment.addOutputModifier("", " * v_Color");
}

sp<Snippet> SnippetTint::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<SnippetTint>::make();
}

}
