#include "renderer/impl/snippet/snippet_alpha.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

void SnippetAlpha::preInitialize(PipelineBuildingContext& context)
{
    context.addPredefinedAttribute("Alpha", "float", Shader::SHADER_STAGE_FRAGMENT);
    context._fragment.addModifier("vec4(1.0, 1.0, 1.0, v_Alpha)");
}

sp<Snippet> SnippetAlpha::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<SnippetAlpha>::make();
}

}
