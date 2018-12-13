#include "renderer/impl/snippet/snippet_acolor.h"

#include "renderer/base/pipeline_layout.h"

namespace ark {

void SnippetAcolor::preInitialize(PipelineLayout& source)
{
    source.addAttribute("Color", "vec4", GLShaderPreprocessor::SHADER_TYPE_FRAGMENT);
}

void SnippetAcolor::preCompile(GraphicsContext& /*graphicsContext*/, PipelineBuildingContext& context)
{
    context._fragment.addModifier("v_Color");
}

sp<Snippet> SnippetAcolor::DICTIONARY::build(const sp<Scope>& /*args*/)
{
    return sp<SnippetAcolor>::make();
}

}
