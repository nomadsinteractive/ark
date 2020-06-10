#include "renderer/impl/snippet/snippet_acolor.h"

#include "renderer/base/pipeline_building_context.h"

namespace ark {

void SnippetAcolor::preInitialize(PipelineBuildingContext& context)
{
    context.addPredefinedAttribute("Color", "vec4", Shader::SHADER_STAGE_FRAGMENT);
    context._fragment.addModifier("v_Color");
}

sp<Snippet> SnippetAcolor::DICTIONARY::build(const Scope& /*args*/)
{
    return sp<SnippetAcolor>::make();
}

}
